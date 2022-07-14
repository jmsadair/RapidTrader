#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

MapOrderBook::MapOrderBook(uint32_t symbol_id_, Concurrent::Messaging::Sender &outgoing_messages_)
    : symbol_id(symbol_id_)
    , outgoing_messages(outgoing_messages_)
{
    last_traded_price = 0;
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
    auto &deleting_order = orders_it->second.order;
    if (notification)
        outgoing_messages.send(DeletedOrder{orders_it->second.order});
    levels_it->second.deleteOrder(deleting_order);
    // Erase the level from the map. Note that this invalidates the iterator to the level.
    if (levels_it->second.empty() && deleting_order.isLimit())
        deleting_order.isAsk() ? ask_levels.erase(levels_it) : bid_levels.erase(levels_it);
    else if (levels_it->second.empty() && (deleting_order.isStop() || deleting_order.isStopLimit()))
        deleting_order.isAsk() ? stop_ask_levels.erase(levels_it) : stop_bid_levels.erase(levels_it);
    // Erase the deleting_order.
    orders.erase(orders_it);
}

void MapOrderBook::replaceOrder(uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
{
    auto orders_it = orders.find(order_id);
    assert(orders_it != orders.end() && "Order does not exist!");
    assert(new_price > 0 && "Price must be positive!");
    assert(new_order_id > 0 && "Order ID must be positive!");
    // Make a copy of the order that is being replaced.
    Order new_order = orders_it->second.order;
    new_order.setOrderID(new_order_id);
    new_order.setPrice(new_price);
    // Remove the order being replaced.
    deleteOrder(order_id);
    // Add the new order.
    addOrder(new_order);
}

void MapOrderBook::addLimitOrder(Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    // Match the order.
    match(order);
    // Insert order into book if it is not filled, not IOC, and not FOK.
    if (!order.isFilled() && !order.isIoc() && !order.isFok())
        insertLimitOrder(order);
    // Otherwise, notify that the order was deleted.
    else
        outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::insertLimitOrder(const Order &order)
{
    assert(order.isLimit() && "Order must be a limit order!");
    if (order.isAsk())
    {
        auto ask_levels_it = ask_levels.find(order.getPrice());
        // Make a new ask level.
        if (ask_levels_it == ask_levels.end())
        {
            auto level_it = ask_levels
                                .emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
                                    std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id))
                                .first;
            auto orders_it = orders.insert({order.getOrderID(), {order, level_it}}).first;
            level_it->second.addOrder(orders_it->second.order);
        }
        // Insert order into existing ask level.
        else
        {
            auto orders_it = orders.insert({order.getOrderID(), {order, ask_levels_it}}).first;
            ask_levels_it->second.addOrder(orders_it->second.order);
        }
    }
    else
    {
        auto bid_levels_it = bid_levels.find(order.getPrice());
        // Make a new bid level.
        if (bid_levels_it == bid_levels.end())
        {
            auto level_it = bid_levels
                                .emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
                                    std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id))
                                .first;
            auto orders_it = orders.insert({order.getOrderID(), {order, level_it}}).first;
            level_it->second.addOrder(orders_it->second.order);
        }
        // Insert order into existing bid level.
        else
        {
            auto orders_it = orders.insert({order.getOrderID(), {order, bid_levels_it}}).first;
            bid_levels_it->second.addOrder(orders_it->second.order);
        }
    }
}

void MapOrderBook::addMarketOrder(Order &order)
{
    assert(order.isMarket() && "Order must be a market order!");
    assert(order.isFok() || order.isIoc() && "Market orders must have IOC or FOK time in force!");
    // Set the price of the order the max/min possible value for matching.
    order.setPrice(order.isAsk() ? 0 : std::numeric_limits<uint64_t>::max());
    match(order);
    outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::addStopOrder(Order &order)
{
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    uint64_t last_traded_bid_price = lastTradedPriceBid();
    uint64_t last_traded_ask_price = lastTradedPriceAsk();
    uint64_t order_stop_price = order.getStopPrice();
    // Activate the stop order if the last traded price satisfies the stop price.
    if ((order.isAsk() && last_traded_bid_price >= order_stop_price) || (order.isBid() && last_traded_ask_price <= order_stop_price))
    {
        // Convert the order to a market order and match it.
        order.setType(order.isStop() ? OrderType::Market : OrderType::Limit);
        outgoing_messages.send(UpdatedOrder{order});
        order.isMarket() ? addMarketOrder(order) : addLimitOrder(order);
        return;
    }
    insertStopOrder(order);
}

void MapOrderBook::insertStopOrder(const Order &order)
{
    assert(order.isStop() || order.isStopLimit() && "Order must be a stop order!");
    if (order.isAsk())
    {
        auto stop_ask_levels_it = stop_ask_levels.find(order.getStopPrice());
        // Make a new ask level.
        if (stop_ask_levels_it == stop_ask_levels.end())
        {
            auto level_it = stop_ask_levels
                                .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                    std::make_tuple(order.getStopPrice(), LevelSide::Ask, symbol_id))
                                .first;
            auto orders_it = orders.insert({order.getOrderID(), {order, level_it}}).first;
            level_it->second.addOrder(orders_it->second.order);
        }
        // Insert order into existing ask level.
        else
        {
            auto orders_it = orders.insert({order.getOrderID(), {order, stop_ask_levels_it}}).first;
            stop_ask_levels_it->second.addOrder(orders_it->second.order);
        }
    }
    else
    {
        auto stop_bid_levels_it = stop_bid_levels.find(order.getStopPrice());
        // Make a new bid level.
        if (stop_bid_levels_it == stop_bid_levels.end())
        {
            auto level_it = stop_bid_levels
                                .emplace(std::piecewise_construct, std::make_tuple(order.getStopPrice()),
                                    std::make_tuple(order.getStopPrice(), LevelSide::Bid, symbol_id))
                                .first;
            auto orders_it = orders.insert({order.getOrderID(), {order, level_it}}).first;
            level_it->second.addOrder(orders_it->second.order);
        }
        // Insert order into existing bid level.
        else
        {
            auto orders_it = orders.insert({order.getOrderID(), {order, stop_bid_levels_it}}).first;
            stop_bid_levels_it->second.addOrder(orders_it->second.order);
        }
    }
}

void MapOrderBook::activateStopOrders()
{
    bool activated_orders = false;
    auto bid_level_it = stop_bid_levels.rbegin();
    auto ask_level_it = stop_ask_levels.begin();
    // Activating stop orders may result in trades which may result in more stop order being activated.
    // Continue activating stop orders until there are none left or none can be activated.
    while (true)
    {
        uint64_t last_ask_price = lastTradedPriceAsk();
        // Activate all bid stop orders that have a price that is at least the last traded price.
        while (bid_level_it != stop_bid_levels.rend() && bid_level_it->first >= last_ask_price)
        {
            activated_orders = true;
            Order &bid_stop_order = bid_level_it->second.front();
            activateStopOrder(bid_stop_order);
            bid_level_it = stop_bid_levels.rbegin();
        }
        uint64_t last_bid_price = lastTradedPriceBid();
        // Activate all ask stop orders that have a price that is at most the last traded price.
        while (ask_level_it != stop_ask_levels.end() && ask_level_it->first <= last_bid_price)
        {
            activated_orders = true;
            Order &ask_stop_order = ask_level_it->second.front();
            activateStopOrder(ask_stop_order);
            ask_level_it = stop_ask_levels.begin();
        }
        if (!activated_orders)
            break;
        activated_orders = false;
    }
}

void MapOrderBook::activateStopOrder(Order order)
{
    // Remove order from book - this is safe since a copy of the order was made.
    // Do not notify that order was deleted.
    deleteOrder(order.getOrderID(), false);
    // Convert the stop order to a limit or market order and match it.
    if (order.isStop())
    {
        order.setType(OrderType::Market);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the market order.
        addMarketOrder(order);
    }
    else
    {
        order.setType(OrderType::Limit);
        // Send notification indicating that stop order has been activated.
        outgoing_messages.send(UpdatedOrder{order});
        // Place the limit order.
        addLimitOrder(order);
    }
}

void MapOrderBook::match(Order &order)
{
    // Order is a FOK order that cannot be filled.
    if (order.isFok() && !canMatchOrder(order))
        return;
    // Order is on the ask side.
    if (order.isAsk())
    {
        // Maximum price bid bid_levels_it.
        auto bid_levels_it = bid_levels.rbegin();
        Order &ask_order = order;
        while (bid_levels_it != bid_levels.rend() && bid_levels_it->first >= ask_order.getPrice() && !ask_order.isFilled())
        {
            Order &bid_order = bid_levels_it->second.front();
            uint64_t executing_price = bid_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            // If the existing order is filled, remove it from the book.
            if (bid_order.isFilled())
                deleteOrder(bid_order.getOrderID(), true);
            // Reset the levels iterator in case it was deleted when the order was deleted.
            bid_levels_it = bid_levels.rbegin();
        }
    }
    // Order is on the bid side.
    if (order.isBid())
    {
        // Minimum price ask ask_levels_it.
        auto ask_levels_it = ask_levels.begin();
        Order &bid_order = order;
        while (ask_levels_it != ask_levels.end() && ask_levels_it->first <= bid_order.getPrice() && !bid_order.isFilled())
        {
            Order &ask_order = ask_levels_it->second.front();
            uint64_t executing_price = ask_order.getPrice();
            executeOrders(ask_order, bid_order, executing_price);
            // If the existing order is filled, remove it from the book.
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
