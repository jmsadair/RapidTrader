#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

MapOrderBook::MapOrderBook(uint32_t symbol_id_, Concurrent::Messaging::Sender &outgoing_messages_)
    : symbol_id(symbol_id_)
    , outgoing_messages(outgoing_messages_)
{
    ask_market_price = std::numeric_limits<uint32_t>::max();
    bid_market_price = 0;
}

void MapOrderBook::addOrder(Order order)
{
    assert(order.getSymbolID() == symbol_id && "Order symbol ID does not match the orderbook symbol ID!");
    // Notify that order has been submitted.
    outgoing_messages.send(AddedOrder{order});
    switch (order.getAction())
    {
    case OrderAction::Limit:
        addLimitOrder(order);
        break;
    case OrderAction::Market:
        addMarketOrder(order);
        break;
    case OrderAction::Stop:
    case OrderAction::StopLimit:
        addStopOrder(order);
        break;
    }
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price)
{
    // Find the order.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order &order = it->second.order;
    auto level_it = it->second.level_it;
    // Calculate the minimum quantity to execute.
    uint64_t executed_quantity = std::min(quantity, order.getOpenQuantity());
    order.execute(price, executed_quantity);
    updateMatchingPrice(order.getLastExecutedPrice());
    outgoing_messages.send(ExecutedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled())
        deleteOrder(order_id, true);
    match();
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity)
{
    // Find the order.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order &order = it->second.order;
    // Calculate the minimum quantity to execute.
    uint64_t execute_quantity = std::min(quantity, order.getOpenQuantity());
    uint32_t execute_price = order.getPrice();
    it->second.order.execute(execute_price, execute_quantity);
    updateMatchingPrice(execute_price);
    outgoing_messages.send(ExecutedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled())
        deleteOrder(order_id, true);
    match();
}

void MapOrderBook::cancelOrder(uint64_t order_id, uint64_t quantity)
{
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order &order = it->second.order;
    order.setQuantity(quantity);
    outgoing_messages.send(UpdatedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled())
        deleteOrder(order_id, true);
    match();
}

void MapOrderBook::deleteOrder(uint64_t order_id)
{
    deleteOrder(order_id, true);
    match();
}

void MapOrderBook::deleteOrder(uint64_t order_id, bool notification)
{
    // Find the order and the iterator to the level it belongs to.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    auto level_it = it->second.level_it;
    if (notification)
        outgoing_messages.send(DeletedOrder{it->second.order});
    // Remove the order from level.
    level_it->second.deleteOrder(it->second.order);
    // Erase the level from the map. Note that this invalidates the iterator to the level.
    if (level_it->second.empty() && it->second.order.isLimit())
        it->second.order.isAsk() ? ask_levels.erase(level_it) : bid_levels.erase(level_it);
    // Erase the level from the map. Note that this invalidates the iterator to the level.
    if (level_it->second.empty() && (it->second.order.isStop() || it->second.order.isStopLimit()))
        it->second.order.isAsk() ? stop_ask_levels.erase(level_it) : stop_bid_levels.erase(level_it);
    // Remove the order wrapper.
    orders.erase(it);
}

void MapOrderBook::replaceOrder(uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order new_order = it->second.order;
    new_order.setOrderID(new_order_id);
    new_order.setPrice(new_price);
    deleteOrder(order_id);
    addOrder(new_order);
    match();
}

void MapOrderBook::addLimitOrder(Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    // Match the order.
    matchLimitOrder(order);
    match();
}

void MapOrderBook::insertLimitOrder(const Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    if (order.isAsk())
    {
        auto level_it = ask_levels.find(order.getPrice());
        // Make a new ask level.
        if (level_it == ask_levels.end())
        {
            auto level_pair = ask_levels.emplace(
                std::piecewise_construct, std::make_tuple(order.getPrice()), std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
            auto order_pair = orders.insert({order.getOrderID(), {order, level_pair.first}});
            level_pair.first->second.addOrder(order_pair.first->second.order);
        }
        // Insert order into existing ask level.
        else
        {
            auto order_pair = orders.insert({order.getOrderID(), {order, level_it}});
            level_it->second.addOrder(order_pair.first->second.order);
        }
    }
    else
    {
        auto level_it = bid_levels.find(order.getPrice());
        // Make a new bid level.
        if (level_it == bid_levels.end())
        {
            auto level_pair = bid_levels.emplace(
                std::piecewise_construct, std::make_tuple(order.getPrice()), std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
            auto order_pair = orders.insert({order.getOrderID(), {order, level_pair.first}});
            level_pair.first->second.addOrder(order_pair.first->second.order);
        }
        // Insert order into existing bid level.
        else
        {
            auto order_pair = orders.insert({order.getOrderID(), {order, level_it}});
            level_it->second.addOrder(order_pair.first->second.order);
        }
    }
}

void MapOrderBook::addMarketOrder(Order &order)
{
    assert(order.isMarket() && "Order must be a market order!");
    matchMarketOrder(order);
    match();
}

void MapOrderBook::addStopOrder(Order &order)
{
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    if ((order.isAsk() && marketPriceBid() >= order.getPrice()) || (order.isBid() && marketPriceAsk() <= order.getPrice()))
    {
        // Convert the order to a market order and match it.
        order.setAction(order.isStop() ? OrderAction::Market : OrderAction::Limit);
        outgoing_messages.send(UpdatedOrder{order});
        order.isMarket() ? addMarketOrder(order) : addLimitOrder(order);
        return;
    }
    insertStopOrder(order);
    match();
}

void MapOrderBook::insertStopOrder(const Order &order)
{
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    if (order.isAsk())
    {
        auto level_it = stop_ask_levels.find(order.getPrice());
        // Make a new ask level.
        if (level_it == stop_ask_levels.end())
        {
            auto level_pair = stop_ask_levels.emplace(
                std::piecewise_construct, std::make_tuple(order.getPrice()), std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
            auto order_pair = orders.insert({order.getOrderID(), {order, level_pair.first}});
            level_pair.first->second.addOrder(order_pair.first->second.order);
        }
        // Insert order into existing ask level.
        else
        {
            auto order_pair = orders.insert({order.getOrderID(), {order, level_it}});
            level_it->second.addOrder(order_pair.first->second.order);
        }
    }
    else
    {
        auto level_it = stop_bid_levels.find(order.getPrice());
        // Make a new bid level.
        if (level_it == stop_bid_levels.end())
        {
            auto level_pair = stop_bid_levels.emplace(
                std::piecewise_construct, std::make_tuple(order.getPrice()), std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
            auto order_pair = orders.insert({order.getOrderID(), {order, level_pair.first}});
            level_pair.first->second.addOrder(order_pair.first->second.order);
        }
        // Insert order into existing bid level.
        else
        {
            auto order_pair = orders.insert({order.getOrderID(), {order, level_it}});
            level_it->second.addOrder(order_pair.first->second.order);
        }
    }
}

bool MapOrderBook::activateStopOrders()
{
    bool activated_orders = false;
    uint32_t market_ask_price = marketPriceAsk();
    uint32_t market_bid_price = marketPriceBid();
    auto bid_level_it = stop_bid_levels.rbegin();
    auto ask_level_it = stop_ask_levels.begin();
    // Activate all bid stop orders that have a price that is greater than the current market ask price.
    while (bid_level_it != stop_bid_levels.rend() && bid_level_it->first >= market_ask_price)
    {
        activated_orders = true;
        Order &bid_stop_order = bid_level_it->second.front();
        activateStopOrder(bid_stop_order);
        bid_level_it = stop_bid_levels.rbegin();
    }
    // Activate all ask stop orders that have a price that is less than current market bid price.
    while (ask_level_it != stop_ask_levels.end() && ask_level_it->second.getPrice() <= market_bid_price)
    {
        activated_orders = true;
        Order &ask_stop_order = ask_level_it->second.front();
        activateStopOrder(ask_stop_order);
        ask_level_it = stop_ask_levels.begin();
    }
    return activated_orders;
}

void MapOrderBook::activateStopOrder(Order order)
{
    // Remove order from book - this is safe since a copy of the order was made.
    // Do not notify that order was deleted.
    deleteOrder(order.getOrderID(), false);
    // Convert the stop order to a limit or market order and match it.
    if (order.isStop())
    {
        order.setAction(OrderAction::Market);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the market order.
        matchMarketOrder(order);
    }
    else
    {
        order.setAction(OrderAction::Limit);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the limit order.
        matchLimitOrder(order);
    }
}

void MapOrderBook::match()
{
    auto bid_levels_it = bid_levels.rbegin();
    auto ask_levels_it = ask_levels.begin();
    while (true)
    {
        while (ask_levels_it != ask_levels.end() && bid_levels_it != bid_levels.rend() && ask_levels_it->first <= bid_levels_it->first)
        {
            Order &min_ask = ask_levels_it->second.front();
            Order &max_bid = bid_levels_it->second.front();
            uint32_t executing_price = min_ask.getOpenQuantity() < max_bid.getOpenQuantity() ? min_ask.getPrice() : max_bid.getPrice();
            // If either orders are AON and cannot be filled, quit.
            if (max_bid.isAon() && !canProcess(max_bid))
                return;
            if (min_ask.isAon() && !canProcess(min_ask))
                return;
            // Match the orders.
            matchOrders(min_ask, max_bid, executing_price);
            bool price_level_change =
                (max_bid.isFilled() && bid_levels_it->second.size() == 1) || (min_ask.isFilled() && ask_levels_it->second.size() == 1);
            // Remove max_bid order if it is filled. May delete current level.
            if (max_bid.isFilled())
                deleteOrder(max_bid.getOrderID(), true);
            // Remove min_ask order if it is filled. May delete current level.
            if (min_ask.isFilled())
                deleteOrder(min_ask.getOrderID(), true);
            // Activate stop orders if price has been updated.
            if (price_level_change)
                activateStopOrders();
            // Reset level iterators in case level was deleted.
            bid_levels_it = bid_levels.rbegin();
            ask_levels_it = ask_levels.begin();
        }
        if (!activateStopOrders())
            break;
    }
}

void MapOrderBook::match(Order &order)
{
    // Quit if the order is AON or FOK and cannot be completely filled.
    if ((order.isAon() || order.isFok()) && !canProcess(order))
        return;
    // Order is on the ask side.
    if (order.isAsk())
    {
        // Maximum price bid level.
        auto level = bid_levels.rbegin();
        while (level != bid_levels.rend() && level->first >= order.getPrice() && !order.isFilled())
        {
            Order &bid = level->second.front();
            uint32_t executing_price = bid.getPrice();
            // If the bid order is AON and cannot be filled by the ask order, quit.
            if (bid.isAon() && bid.getOpenQuantity() > order.getOpenQuantity())
                return;
            matchOrders(order, bid, executing_price);
            // If the existing bid order is filled, remove it from the book.
            if (bid.isFilled())
                deleteOrder(bid.getOrderID(), true);
            level = bid_levels.rbegin();
        }
    }
    // Order is on the bid side.
    if (order.isBid())
    {
        // Minimum price ask level.
        auto level = ask_levels.begin();
        while (level != ask_levels.end() && level->first <= order.getPrice() && !order.isFilled())
        {
            Order &ask = level->second.front();
            uint32_t executing_price = ask.getPrice();
            // If the ask order is AON and cannot be filled by the bid order, quit.
            if (ask.isAon() && ask.getOpenQuantity() > order.getOpenQuantity())
                return;
            matchOrders(ask, order, executing_price);
            // If the existing ask order is filled, remove it from the book.
            if (ask.isFilled())
                deleteOrder(ask.getOrderID(), true);
            level = ask_levels.begin();
        }
    }
}

void MapOrderBook::matchOrders(Order &ask, Order &bid, uint32_t price)
{
    assert(ask.isAsk() && bid.isBid() && "Cannot match orders on same side!");
    // Calculate the minimum quantity to match.
    uint64_t matched_quantity = std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(price, matched_quantity);
    ask.execute(price, matched_quantity);
    // Send order execution notifications for the matched orders.
    outgoing_messages.send(ExecutedOrder{bid});
    outgoing_messages.send(ExecutedOrder{ask});
    // Update market price
    updateMatchingPrice(price);
}

bool MapOrderBook::canProcess(const Order &order) const
{
    // An order cannot be matched if there are no orders in the book.
    if (orders.empty())
        return false;
    // The quantity of the order that is able to be filled.
    uint64_t quantity_can_fill = 0;
    if (order.isAsk())
    {
        auto level = bid_levels.rbegin();
        while (quantity_can_fill < order.getOpenQuantity() && level != bid_levels.rend() && level->first >= order.getPrice())
        {
            const auto &level_orders = level->second.getOrders();
            for (const auto &level_order : level_orders)
            {
                // If level order is AON and there is not enough remaining quantity of order to fill it, quit.
                if (level_order.isAon() && order.getOpenQuantity() - quantity_can_fill < level_order.getOpenQuantity())
                    return quantity_can_fill >= order.getOpenQuantity();
                quantity_can_fill += level_order.getOpenQuantity();
            }
            ++level;
        }
    }
    else
    {
        auto level = ask_levels.begin();
        while (quantity_can_fill < order.getOpenQuantity() && level != ask_levels.end() && level->first <= order.getPrice())
        {
            const auto &level_orders = level->second.getOrders();
            for (const auto &level_order : level_orders)
            {
                // If level order is AON and there is not enough remaining quantity of order to fill it, quit.
                if (level_order.isAon() && order.getOpenQuantity() - quantity_can_fill < level_order.getOpenQuantity())
                    return quantity_can_fill >= order.getOpenQuantity();
                quantity_can_fill += level_order.getOpenQuantity();
            }
            ++level;
        }
    }
    return quantity_can_fill >= order.getOpenQuantity();
}
void MapOrderBook::matchLimitOrder(Order &order)
{
    match(order);
    // Insert order into book if it is not filled, not IOC, and not FOK.
    if (!order.isFilled() && !order.isIoc() && !order.isFok())
        insertLimitOrder(order);
    // Otherwise, notify that the order was deleted.
    else
        outgoing_messages.send(DeletedOrder{order});
}
void MapOrderBook::matchMarketOrder(Order &order)
{
    // Set the price of the order the max/min possible value for matching.
    order.setPrice(order.isAsk() ? 0 : std::numeric_limits<uint32_t>::max());
    match(order);
    outgoing_messages.send(DeletedOrder{order});
}
