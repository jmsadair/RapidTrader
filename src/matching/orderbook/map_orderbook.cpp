#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

MapOrderBook::MapOrderBook(uint32_t symbol_id_, Concurrent::Messaging::Sender &outgoing_messages_)
    : symbol_id(symbol_id_)
    , outgoing_messages(outgoing_messages_)
{
    last_traded_price = 0;
    previous_last_traded_price = 0;
}

void MapOrderBook::addOrder(Order order)
{
    assert(order.getSymbolID() == symbol_id && "Order symbol ID does not match the orderbook symbol ID!");
    // Notify that order has been submitted.
    outgoing_messages.send(AddedOrder{order});
    // Add the order to the book.
    switch (order.getType())
    {
    case OrderType::Limit:
        addLimitOrder(order);
        break;
    case OrderType::Market:
        addMarketOrder(order);
        break;
    case OrderType::Stop:
    case OrderType::StopLimit:
    case OrderType::TrailingStop:
    case OrderType::TrailingStopLimit:
        addStopOrder(order);
        break;
    }
    // Try to activate stop orders.
    activateStopOrders();
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity, uint64_t price)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    assert(price > 0 && "Price must be positive!");
    assert(quantity > 0 && "Quantity must be positive!");
    Order &executing_order = orders_it->second.order;
    uint64_t executing_quantity = std::min(quantity, executing_order.getOpenQuantity());
    executing_order.execute(price, executing_quantity);
    previous_last_traded_price = last_traded_price;
    last_traded_price = price;
    outgoing_messages.send(ExecutedOrder{executing_order});
    if (executing_order.isFilled())
        deleteOrder(order_id, true);
    activateStopOrders();
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    assert(quantity > 0 && "Quantity must be positive!");
    Order &executing_order = orders_it->second.order;
    uint64_t executing_quantity = std::min(quantity, executing_order.getOpenQuantity());
    uint64_t executing_price = executing_order.getPrice();
    executing_order.execute(executing_price, executing_quantity);
    previous_last_traded_price = last_traded_price;
    last_traded_price = executing_price;
    outgoing_messages.send(ExecutedOrder{executing_order});
    if (executing_order.isFilled())
        deleteOrder(order_id, true);
    activateStopOrders();
}

void MapOrderBook::cancelOrder(uint64_t order_id, uint64_t quantity)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    assert(quantity > 0 && "Quantity must be positive!");
    Order &cancelling_order = orders_it->second.order;
    cancelling_order.setQuantity(quantity);
    outgoing_messages.send(UpdatedOrder{cancelling_order});
    if (cancelling_order.isFilled())
        deleteOrder(order_id, true);
}

void MapOrderBook::deleteOrder(uint64_t order_id)
{
    deleteOrder(order_id, true);
}

void MapOrderBook::deleteOrder(uint64_t order_id, bool notification)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    auto &levels_it = orders_it->second.level_it;
    Order &deleting_order = orders_it->second.order;
    if (notification)
        outgoing_messages.send(DeletedOrder{orders_it->second.order});
    // Erase the order from the level.
    levels_it->second.deleteOrder(deleting_order);
    // Erase the level from the map. Note that this invalidates the iterator to the level.
    if (levels_it->second.empty())
    {
        switch (deleting_order.getType())
        {
        case OrderType::Limit:
            deleting_order.isAsk() ? ask_levels.erase(levels_it) : bid_levels.erase(levels_it);
            break;
        case OrderType::Stop:
        case OrderType::StopLimit:
            deleting_order.isAsk() ? stop_ask_levels.erase(levels_it) : stop_bid_levels.erase(levels_it);
            break;
        case OrderType::TrailingStop:
        case OrderType::TrailingStopLimit:
            deleting_order.isAsk() ? trailing_stop_ask_levels.erase(levels_it) : trailing_stop_bid_levels.erase(levels_it);
            break;
        default:
            assert(false && "Invalid order type!");
        }
    }
    // Erase the order from the unordered map.
    orders.erase(orders_it);
}

void MapOrderBook::replaceOrder(uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    assert(new_price > 0 && "Price must be positive!");
    assert(new_order_id > 0 && "Order ID must be positive!");
    Order new_order = orders_it->second.order;
    new_order.setOrderID(new_order_id);
    new_order.setPrice(new_price);
    deleteOrder(order_id);
    addOrder(new_order);
}

void MapOrderBook::addLimitOrder(Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    match(order);
    if (!order.isFilled() && !order.isIoc() && !order.isFok())
        insertLimitOrder(order);
    else
        outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::insertLimitOrder(const Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    if (order.isAsk())
    {
        auto level_it = ask_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
                                std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
    else
    {
        auto level_it = bid_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
                                std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
}

void MapOrderBook::addMarketOrder(Order &order)
{
    assert(order.isMarket() && "Order must be a market order!");
    assert(order.isFok() || order.isIoc() && "Market orders must have IOC or FOK time in force!");
    order.setPrice(order.isAsk() ? 0 : std::numeric_limits<uint64_t>::max());
    match(order);
    outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::addStopOrder(Order &order)
{
    uint64_t last_traded_bid_price = lastTradedPriceBid();
    uint64_t last_traded_ask_price = lastTradedPriceAsk();
    uint64_t order_stop_price = order.getStopPrice();
    // Activate the stop order if the last traded price satisfies the stop price.
    if ((order.isAsk() && last_traded_bid_price <= order_stop_price) || (order.isBid() && last_traded_ask_price >= order_stop_price))
    {
        // Convert the order to a market order and match it.
        order.setType((order.isStop() || order.isTrailingStop()) ? OrderType::Market : OrderType::Limit);
        outgoing_messages.send(UpdatedOrder{order});
        order.isMarket() ? addMarketOrder(order) : addLimitOrder(order);
        return;
    }
    order.isStop() || order.isStopLimit() ? insertStopOrder(order) : insertTrailingStopOrder(order);
}

void MapOrderBook::insertStopOrder(const Order &order)
{
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    if (order.isAsk())
    {
        auto level_it = stop_ask_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                std::make_tuple(order.getStopPrice(), LevelSide::Ask, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
    else
    {
        auto level_it = stop_bid_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                std::make_tuple(order.getStopPrice(), LevelSide::Bid, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
    }
}

void MapOrderBook::insertTrailingStopOrder(const Order &order)
{
    assert(order.isTrailingStop() || order.isTrailingStopLimit() && "Order must be a trailing stop order!");
    if (order.isAsk())
    {
        auto level_it = trailing_stop_ask_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                std::make_tuple(order.getStopPrice(), LevelSide::Ask, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
        orders_it->second.level_it = level_it;
    }
    else
    {
        auto level_it = trailing_stop_bid_levels
                            .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                std::make_tuple(order.getStopPrice(), LevelSide::Bid, symbol_id))
                            .first;
        auto [orders_it, success] = orders.emplace(order.getOrderID(), OrderWrapper{order, level_it});
        level_it->second.addOrder(orders_it->second.order);
        orders_it->second.level_it = level_it;
    }
}

void MapOrderBook::activateStopOrders()
{
    bool stop = true;
    // Activating stop orders may result in trades which may result in more stop order being activated.
    // Continue activating stop orders until there are none left or none can be activated.
    while (stop)
    {
        stop = activateBidStopOrders();
        updateAskStopOrders();
        stop = stop || activateAskStopOrders();
        updateBidStopOrders();
    }
}

bool MapOrderBook::activateBidStopOrders()
{
    bool activated_orders = false;
    auto stop_levels_it = stop_bid_levels.begin();
    uint64_t last_ask_price = lastTradedPriceAsk();
    // Starting with the stop order on the bid side at the lowest stop price, check if the stop
    // price is at most the last traded price. If it is, activate the stop order.
    while (stop_levels_it != stop_bid_levels.end() && stop_levels_it->first <= last_ask_price)
    {
        activated_orders = true;
        Order &stop_order = stop_levels_it->second.front();
        activateStopOrder(stop_order);
        stop_levels_it = stop_bid_levels.begin();
    }
    // Repeat for trailing stops.
    auto trailing_stop_levels_it = trailing_stop_bid_levels.begin();
    last_ask_price = lastTradedPriceAsk();
    while (trailing_stop_levels_it != trailing_stop_bid_levels.end() && trailing_stop_levels_it->first <= last_ask_price)
    {
        activated_orders = true;
        Order &trailing_stop_order = trailing_stop_levels_it->second.front();
        activateStopOrder(trailing_stop_order);
        trailing_stop_levels_it = trailing_stop_bid_levels.begin();
    }
    return activated_orders;
}

bool MapOrderBook::activateAskStopOrders()
{
    bool activated_orders = false;
    uint64_t last_bid_price = lastTradedPriceBid();
    auto stop_levels_it = stop_ask_levels.rbegin();
    // Starting with the stop order on the ask side at the highest stop price, check if the stop
    // price is at least the last traded price. If it is, activate the stop order.
    while (stop_levels_it != stop_ask_levels.rend() && stop_levels_it->first >= last_bid_price)
    {
        activated_orders = true;
        Order &stop_order = stop_levels_it->second.front();
        activateStopOrder(stop_order);
        stop_levels_it = stop_ask_levels.rbegin();
    }
    // Repeat for trailing stops.
    auto trailing_stop_levels_it = trailing_stop_ask_levels.rbegin();
    last_bid_price = lastTradedPriceBid();
    while (trailing_stop_levels_it != trailing_stop_ask_levels.rend() && trailing_stop_levels_it->first >= last_bid_price)
    {
        activated_orders = true;
        Order &trailing_stop_order = trailing_stop_levels_it->second.front();
        activateStopOrder(trailing_stop_order);
        trailing_stop_levels_it = trailing_stop_ask_levels.rbegin();
    }
    return activated_orders;
}

void MapOrderBook::activateStopOrder(Order order)
{
    // Remove order from book - this is safe since a copy of the order was made.
    // Do not notify that order was deleted.
    deleteOrder(order.getOrderID(), false);
    // Convert the stop / trailing stop order to a limit or market order and match it.
    if (order.isStop() || order.isTrailingStop())
    {
        order.setType(OrderType::Market);
        outgoing_messages.send(UpdatedOrder{order});
        addMarketOrder(order);
    }
    else
    {
        order.setType(OrderType::Limit);
        outgoing_messages.send(UpdatedOrder{order});
        addLimitOrder(order);
    }
}

void MapOrderBook::updateBidStopOrders()
{
    if (previousLastTradedPriceAsk() <= lastTradedPriceAsk())
        return;
    std::map<uint64_t, Level> new_trailing_levels;
    uint64_t stop_price_decrease = previousLastTradedPriceAsk() - lastTradedPriceAsk();
    auto trailing_levels_it = trailing_stop_bid_levels.begin();
    while (trailing_levels_it != trailing_stop_bid_levels.end())
    {
        uint64_t stop_price = trailing_levels_it->first;
        uint64_t new_stop_price = stop_price_decrease >= stop_price ? 1 : stop_price - stop_price_decrease;
        auto new_trailing_levels_it = new_trailing_levels.emplace_hint(new_trailing_levels.end(), std::piecewise_construct,
            std::make_tuple(new_stop_price), std::make_tuple(new_stop_price, LevelSide::Bid, symbol_id));
        while (!trailing_levels_it->second.empty())
        {
            // Get the first order.
            Order &stop_order = trailing_levels_it->second.front();
            // Update the stop price of the order.
            stop_order.setStopPrice(new_stop_price);
            // Remove the order from the current level.
            trailing_levels_it->second.popFront();
            // Update the map iterator associated with the order.
            orders.find(stop_order.getOrderID())->second.level_it = new_trailing_levels_it;
            // Add it to the new level.
            new_trailing_levels_it->second.addOrder(stop_order);
            // Notify that the order's stop price has been adjusted.
            outgoing_messages.send(UpdatedOrder{stop_order});
        }
        ++trailing_levels_it;
    }
    std::swap(trailing_stop_bid_levels, new_trailing_levels);
    // Update the previous last traded price.
    previous_last_traded_price = last_traded_price;
}

void MapOrderBook::updateAskStopOrders()
{
    if (previousLastTradedPriceBid() >= lastTradedPriceBid() || trailing_stop_ask_levels.empty())
        return;
    std::map<uint64_t, Level> new_trailing_levels;
    uint64_t stop_price_increase = lastTradedPriceBid() - previousLastTradedPriceBid();
    auto trailing_levels_it = trailing_stop_ask_levels.begin();
    while (trailing_levels_it != trailing_stop_ask_levels.end())
    {
        uint64_t stop_price = trailing_levels_it->first;
        uint64_t new_stop_price = stop_price + stop_price_increase;
        auto new_trailing_levels_it = new_trailing_levels.emplace_hint(new_trailing_levels.end(), std::piecewise_construct,
            std::make_tuple(new_stop_price), std::make_tuple(new_stop_price, LevelSide::Ask, symbol_id));
        while (!trailing_levels_it->second.empty())
        {
            // Get the first order.
            Order &stop_order = trailing_levels_it->second.front();
            // Update the stop price of the order.
            stop_order.setStopPrice(new_stop_price);
            // Remove the order from the current level.
            trailing_levels_it->second.popFront();
            // Update the map iterator associated with the order.
            orders.find(stop_order.getOrderID())->second.level_it = new_trailing_levels_it;
            // Add it to the new level.
            new_trailing_levels_it->second.addOrder(stop_order);
            // Notify that the order's stop price has been adjusted.
            outgoing_messages.send(UpdatedOrder{stop_order});
        }
        ++trailing_levels_it;
    }
    std::swap(trailing_stop_ask_levels, new_trailing_levels);
    // Update the previous last traded price.
    previous_last_traded_price = last_traded_price;
}

void MapOrderBook::match(Order &order)
{
    // Order is a FOK order that cannot be filled.
    if (order.isFok() && !canMatchOrder(order))
        return;
    // Order is on the ask side.
    if (order.isAsk())
    {
        // Maximum price bid level.
        auto bid_levels_it = bid_levels.rbegin();
        Order &ask_order = order;
        while (bid_levels_it != bid_levels.rend() && bid_levels_it->first >= ask_order.getPrice() && !ask_order.isFilled())
        {
            Order &bid_order = bid_levels_it->second.front();
            uint64_t executing_price = bid_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            if (bid_order.isFilled())
                deleteOrder(bid_order.getOrderID(), true);
            // Reset the levels iterator in case it was deleted when the order was deleted.
            bid_levels_it = bid_levels.rbegin();
        }
    }
    // Order is on the bid side.
    if (order.isBid())
    {
        // Minimum price ask level.
        auto ask_levels_it = ask_levels.begin();
        Order &bid_order = order;
        while (ask_levels_it != ask_levels.end() && ask_levels_it->first <= bid_order.getPrice() && !bid_order.isFilled())
        {
            Order &ask_order = ask_levels_it->second.front();
            uint64_t executing_price = ask_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            if (ask_order.isFilled())
                deleteOrder(ask_order.getOrderID(), true);
            // Reset the levels iterator in case it was deleted when the order was deleted.
            ask_levels_it = ask_levels.begin();
        }
    }
}

void MapOrderBook::executeOrders(Order &ask, Order &bid, uint64_t executing_price)
{
    assert(ask.isAsk() && bid.isBid() && "Cannot match orders on same side!");
    // Calculate the minimum quantity to match.
    uint64_t matched_quantity = std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(executing_price, matched_quantity);
    ask.execute(executing_price, matched_quantity);
    // Send order execution notifications for the matched orders.
    outgoing_messages.send(ExecutedOrder{bid});
    outgoing_messages.send(ExecutedOrder{ask});
    // Update last traded price.
    previous_last_traded_price = last_traded_price;
    last_traded_price = executing_price;
}

bool MapOrderBook::canMatchOrder(const Order &order) const
{
    uint64_t price = order.getPrice();
    uint64_t quantity_required = order.getOpenQuantity();
    uint64_t quantity_available = 0;
    if (order.isAsk())
    {
        auto bid_levels_it = bid_levels.rbegin();
        while (bid_levels_it != bid_levels.rend() && bid_levels_it->first >= price)
        {
            uint64_t level_volume = bid_levels_it->second.getVolume();
            uint64_t quantity_needed = quantity_required - quantity_available;
            quantity_available += std::min(level_volume, quantity_needed);
            if (quantity_available >= quantity_required)
                return true;
        }
    }
    else
    {
        auto ask_levels_it = ask_levels.begin();
        while (ask_levels_it != ask_levels.end() && ask_levels_it->first <= price)
        {
            uint64_t level_volume = ask_levels_it->second.getVolume();
            uint64_t quantity_needed = quantity_required - quantity_available;
            quantity_available += std::min(level_volume, quantity_needed);
            if (quantity_available >= quantity_required)
                return true;
        }
    }
    return false;
}