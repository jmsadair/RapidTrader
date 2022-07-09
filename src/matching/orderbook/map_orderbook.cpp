#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

MapOrderBook::MapOrderBook(uint32_t symbol_id_, Concurrent::Messaging::Sender &outgoing_messages_)
    : symbol_id(symbol_id_), outgoing_messages(outgoing_messages_)
{}

void MapOrderBook::addOrder(Order order) {
    assert(order.getSymbolID() == symbol_id && "Order symbol ID does not match the orderbook symbol ID!");
    // Notify that order has been submitted.
    outgoing_messages.send(AddedOrder{order});
    switch(order.getAction())
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
    processMatching();
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price)
{
    // Find the order.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order& order = it->second.order;
    auto level_it = it->second.level_it;
    // Calculate the minimum quantity to execute.
    uint64_t executed_quantity = std::min(quantity, order.getOpenQuantity());
    order.execute(price, executed_quantity);
    outgoing_messages.send(ExecutedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled()) {
        outgoing_messages.send(DeletedOrder{order});
        level_it->second.deleteOrder(it->second.order);
        if (level_it->second.empty())
            order.isLimit() ? deleteLimitLevel(order) : deleteStopLevel(order);
        orders.erase(it);
    }
    processMatching();
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity)
{
    // Find the order.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order& order = it->second.order;
    auto level_it = it->second.level_it;
    // Calculate the minimum quantity to execute.
    uint64_t execute_quantity = std::min(quantity, order.getOpenQuantity());
    it->second.order.execute(order.getPrice(), execute_quantity);
    outgoing_messages.send(ExecutedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled()) {
        outgoing_messages.send(DeletedOrder{order});
        level_it->second.deleteOrder(it->second.order);
        if (level_it->second.empty())
            order.isLimit() ? deleteLimitLevel(order) : deleteStopLevel(order);
        orders.erase(it);
    }
    processMatching();
}

void MapOrderBook::cancelOrder(uint64_t order_id, uint64_t quantity)
{
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    Order& order = it->second.order;
    auto level_it = it->second.level_it;
    order.cancel(quantity);
    outgoing_messages.send(UpdatedOrder{order});
    // If the order is filled, remove it from the book.
    if (order.isFilled()) {
        outgoing_messages.send(DeletedOrder{order});
        level_it->second.deleteOrder(it->second.order);
        if (level_it->second.empty())
            order.isLimit() ? deleteLimitLevel(order) : deleteStopLevel(order);
        orders.erase(it);
    }
    processMatching();
}

void MapOrderBook::deleteOrder(uint64_t order_id)
{
    // Find the order and the iterator to the level it belongs to.
    auto it = orders.find(order_id);
    assert(it != orders.end() && "Order does not exist!");
    // Notify that order has been deleted.
    outgoing_messages.send(DeletedOrder{it->second.order});
    auto level_it = it->second.level_it;
    // Remove the order from level.
    level_it->second.deleteOrder(it->second.order);
    // Remove the limit price level if it is empty.
    if (level_it->second.empty() && it->second.order.isLimit())
        deleteLimitLevel(it->second.order);
    // Remove the stop price level if it is empty.
    if (level_it->second.empty() && it->second.order.isStop())
        deleteStopLevel(it->second.order);
    // Remove the order wrapper.
    orders.erase(it);
    processMatching();
}

void MapOrderBook::addLimitOrder(Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    // Match the order.
    processMatching(order);
    // Insert order into book if it is not filled, not IOC, and not FOK.
    if (!order.isFilled() && !order.isIoc() && !order.isFok())
        insertLimitOrder(order);
    // Otherwise, notify that the order was deleted.
    else
        outgoing_messages.send(DeletedOrder{order});
}

std::map<uint32_t, Level>::iterator MapOrderBook::addLimitLevel(const Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    // Add a level on the ask side.
    if (order.isAsk())
    {
        auto [it, success] = ask_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
         return it;
    }
    // Add a level on the bid side.
    else
    {
        auto [it, success] = bid_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
        return it;
    }
}

void MapOrderBook::deleteLimitLevel(const Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    // Find the iterator to the level that the order belongs.
    auto it = orders.find(order.getOrderID());
    auto level_it = it->second.level_it;
    // Erase the level from the map. Note that this invalidates the iterator.
    order.isAsk() ? ask_levels.erase(level_it) : bid_levels.erase(level_it);
}

void MapOrderBook::insertLimitOrder(const Order &order) {
    assert(order.isLimit() && "Order must be a limit order!");
    auto level_it = order.isAsk() ? ask_levels.find(order.getPrice()) : bid_levels.find(order.getPrice());
    if (level_it == (order.isAsk() ? ask_levels.end() : bid_levels.end()))
    {
        auto new_level_it = addLimitLevel(order);
        auto [it, success] = orders.insert({order.getOrderID(), {order, new_level_it}});
        new_level_it->second.addOrder(it->second.order);
    }
    else
    {
        auto [it, success] = orders.insert({order.getOrderID(), {order, level_it}});
        level_it->second.addOrder(it->second.order);
    }
}

void MapOrderBook::addMarketOrder(Order &order) {
    assert(order.isMarket() && "Order must be a market order!");
    // Set the price of the order the max/min possible value for matching.
    order.setPrice(order.isAsk() ? 0 : std::numeric_limits<uint32_t>::max());
    processMatching(order);
    outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::addStopOrder(Order &order) {
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    if ((order.isAsk() && marketBidPrice() >= order.getPrice()) || (order.isBid() && marketAskPrice() <= order.getPrice())) {
        // Convert the order to a market order and match it.
        order.setAction(order.isStop() ? OrderAction::Market : OrderAction::Limit);
        outgoing_messages.send(UpdatedOrder{order});
        order.isMarket() ? addMarketOrder(order) : addLimitOrder(order);
        return;
    }
    insertStopOrder(order);
}

std::map<uint32_t, Level>::iterator MapOrderBook::addStopLevel(const Order &order)
{
    // Add a level on the ask side.
    if (order.isAsk())
    {
        auto [it, success] = stop_ask_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
        return it;
    }
    // Add a level on the bid side.
    else
    {
        auto [it, success] = stop_bid_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
        return it;
    }
}

void MapOrderBook::deleteStopLevel(const Order &order) {
    // Find the iterator to the stop level that the order belongs.
    auto it = orders.find(order.getOrderID());
    auto level_it = it->second.level_it;
    // Erase the level from the map. Note that this invalidates the iterator.
    order.isAsk() ? stop_ask_levels.erase(level_it) : stop_bid_levels.erase(level_it);
}

void MapOrderBook::insertStopOrder(const Order &order) {
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    auto level_it = order.isAsk() ? stop_ask_levels.find(order.getPrice()) : stop_bid_levels.find(order.getPrice());
    if (level_it == (order.isAsk() ? stop_ask_levels.end() : stop_bid_levels.end()))
    {
        auto new_level_it = addStopLevel(order);
        auto [it, success] = orders.insert({order.getOrderID(), {order, new_level_it}});
        new_level_it->second.addOrder(it->second.order);
    }
    else
    {
        auto [it, success] = orders.insert({order.getOrderID(), {order, level_it}});
        level_it->second.addOrder(it->second.order);
    }
}

void MapOrderBook::activateStopOrders() {
    uint32_t market_ask_price = marketAskPrice();
    uint32_t market_bid_price = marketBidPrice();
    auto bid_level_it = stop_bid_levels.rbegin();
    auto ask_level_it = stop_ask_levels.begin();
    // Activate all bid stop orders that have a price that is greater than the current market ask price.
    while (bid_level_it != stop_bid_levels.rend() && bid_level_it->first >= market_ask_price) {
        Order& bid_stop_order = bid_level_it->second.front();
        activateStopOrder(bid_stop_order);
        bid_level_it = stop_bid_levels.rbegin();
    }
    // Activate all ask stop orders that have a price that is less than current market bid price.
    while (ask_level_it != stop_ask_levels.end() && ask_level_it->second.getPrice() <= market_bid_price) {
        Order& ask_stop_order = ask_level_it->second.front();
        activateStopOrder(ask_stop_order);
        ask_level_it = stop_ask_levels.begin();
    }
}

void MapOrderBook::activateStopOrder(Order order) {
    // Remove order from book - this is safe since a copy of the order was made.
    auto it = orders.find(order.getOrderID());
    auto level_it = it->second.level_it;
    level_it->second.deleteOrder(order);
    if (level_it->second.empty())
        deleteStopLevel(order);
    orders.erase(it);
    // Convert the stop order to a limit or market order and match it.
    if (order.isStop())
    {
        order.setAction(OrderAction::Market);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the market order.
        addMarketOrder(order);
    } else {
        order.setAction(OrderAction::Limit);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the limit order.
        addLimitOrder(order);
    }
}

void MapOrderBook::processMatching()
{
    auto bid_level = bid_levels.rbegin();
    auto ask_level = ask_levels.begin();
    uint32_t last_ask_price = marketAskPrice();
    uint32_t last_bid_price = marketBidPrice();
    while (ask_level != ask_levels.end() && bid_level != bid_levels.rend() && ask_level->first <= bid_level->first)
    {
        // Match the orders.
        Order &ask = ask_level->second.front();
        Order &bid = bid_level->second.front();
        if (bid.isAon() && !canProcess(bid))
            return;
        if (ask.isAon() && !canProcess(ask))
            return;
        matchOrders(ask, bid);
        // Send order execution notifications for the matched orders.
        outgoing_messages.send(ExecutedOrder{bid});
        outgoing_messages.send(ExecutedOrder{ask});
        // Remove the orders from the book if they are filled.
        if (bid.isFilled())
        {
            outgoing_messages.send(DeletedOrder{bid});
            bid_level->second.popFront();
            // Remove the level if it is empty.
            if (bid_level->second.empty())
                deleteLimitLevel(bid);
            orders.erase(bid.getOrderID());

        }
        if (ask.isFilled())
        {
            outgoing_messages.send(DeletedOrder{ask});
            ask_level->second.popFront();
            // Remove the level if it is empty.
            if (ask_level->second.empty())
                deleteLimitLevel(ask);
            orders.erase(ask.getOrderID());
        }
        // If the market price has changed, activate stop orders.
        if (last_ask_price != marketAskPrice() || last_bid_price != marketBidPrice()) {
            activateStopOrders();
            last_ask_price = marketAskPrice();
            last_bid_price = marketBidPrice();
        }
        bid_level = bid_levels.rbegin();
        ask_level = ask_levels.begin();
    }
    activateStopOrders();
}

void MapOrderBook::processMatching(Order &order)
{
    bool can_match = !(order.isAon() || order.isFok()) || canProcess(order);
    // Order is on the ask side.
    if (order.isAsk() && can_match)
    {
        // Maximum price bid level.
        auto level = bid_levels.rbegin();
        while (level != bid_levels.rend() && level->first >= order.getPrice() && !order.isFilled())
        {
            Order &bid = level->second.front();
            // If the bid order is AON and cannot be filled by the ask order, quit.
            if (bid.isAon() && bid.getOpenQuantity() > order.getOpenQuantity())
                return;
            matchOrders(order, bid);
            // Send order execution notifications for the matched orders.
            outgoing_messages.send(ExecutedOrder{bid});
            outgoing_messages.send(ExecutedOrder{order});
            // If the existing bid order is filled, remove it from the book.
            if (bid.isFilled())
            {
                outgoing_messages.send(DeletedOrder{bid});
                level->second.popFront();
                if (level->second.empty())
                    deleteLimitLevel(bid);
                orders.erase(bid.getOrderID());
                level = bid_levels.rbegin();
            }
        }
    }
    // Order is on the bid side.
    if (order.isBid() && can_match)
    {
        // Minimum price ask level.
        auto level = ask_levels.begin();
        while (level != ask_levels.end() && level->first <= order.getPrice() && !order.isFilled())
        {
            Order &ask = level->second.front();
            // If the ask order is AON and cannot be filled by the bid order, quit.
            if (ask.isAon() && ask.getOpenQuantity() > order.getOpenQuantity())
                 return;
            matchOrders(ask, order);
            // Send order execution notifications for the matched orders.
            outgoing_messages.send(ExecutedOrder{order});
            outgoing_messages.send(ExecutedOrder{ask});
            // If the existing ask order is filled, remove it from the book.
            if (ask.isFilled())
            {
                outgoing_messages.send(DeletedOrder{ask});
                level->second.popFront();
                if (level->second.empty())
                    deleteLimitLevel(ask);
                orders.erase(ask.getOrderID());
                level = ask_levels.begin();
            }
        }
    }
}

void MapOrderBook::matchOrders(Order &ask, Order &bid)
{
    assert(ask.isAsk() && bid.isBid() && "Cannot match orders on same side!");
    // Calculate the minimum quantity to match.
    uint64_t matched_quantity = std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(ask.isMarket() ? bid.getPrice() : ask.getPrice(), matched_quantity);
    ask.execute(bid.isMarket() ? ask.getPrice() : bid.getPrice(), matched_quantity);
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
        while (quantity_can_fill < order.getOpenQuantity() && level != bid_levels.rend())
        {
            if (level->second.getPrice() < order.getPrice())
                break;
            quantity_can_fill += level->second.getVolume();
            ++level;
        }
    }
    else
    {
        auto level = ask_levels.begin();
        while (quantity_can_fill < order.getOpenQuantity() && level != ask_levels.end())
        {
            if (order.getPrice() != 0 && level->second.getPrice() > order.getPrice())
                break;
            quantity_can_fill += level->second.getVolume();
            ++level;
        }
    }
    return quantity_can_fill >= order.getQuantity();
}

