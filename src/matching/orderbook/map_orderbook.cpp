#include "map_orderbook.h"
#include "log.h"

// Only check the order book invariants in debug mode.
#ifndef NDEBUG
#    define ORDERBOOK_CHECK_INVARIANTS verifyOrderBookState()
#else
#    define ORDERBOOK_CHECK_INVARIANTS
#endif

MapOrderBook::MapOrderBook(uint32_t symbol_id_)
    : symbol_id(symbol_id_)
{}

void MapOrderBook::addOrder(const Order &order)
{
    auto [it, success] = orders.insert({order.getOrderID(), order});
    if (order.isAsk())
    {
        auto level_it = ask_levels.find(order.getPrice());
        if (level_it == ask_levels.end())
        {
            Level *level_ptr = addLevel(order);
            level_ptr->addOrder(it->second);
        }
        else
        {
            level_it->second.addOrder(it->second);
        }
    }
    else
    {
        auto level_it = bid_levels.find(order.getPrice());
        if (level_it == bid_levels.end())
        {
            Level *level_ptr = addLevel(order);
            level_ptr->addOrder(it->second);
        }
        else
        {
            level_it->second.addOrder(it->second);
        }
    }
}

Level *MapOrderBook::addLevel(const Order &order)
{
    if (order.isAsk())
    {
        auto [it, success] = ask_levels.emplace(order.getPrice(), order.getPrice());
        Level *level_ptr = &it->second;
        return level_ptr;
    }
    else
    {
        auto [it, success] = bid_levels.emplace(order.getPrice(), order.getPrice());
        Level *level_ptr = &it->second;
        return level_ptr;
    }
}

void MapOrderBook::deleteOrder(uint64_t order_id)
{
    auto it = orders.find(order_id);
    auto price_level_it = it->second.isAsk() ? ask_levels.find(it->second.getPrice()) : bid_levels.find(it->second.getPrice());
    price_level_it->second.deleteOrder(it->second);
    if (price_level_it->second.empty())
        deleteLevel(it->second);
    orders.erase(it);
}

void MapOrderBook::deleteLevel(const Order &order)
{
    if (order.isAsk())
        ask_levels.erase(order.getPrice());
    else
        bid_levels.erase(order.getPrice());
}

bool MapOrderBook::processMatching(std::queue<Order> &processed_orders)
{
    while (true)
    {
        auto ask_level = ask_levels.begin();
        auto bid_level = bid_levels.rbegin();
        uint8_t halt_processing = ask_levels.empty() + bid_levels.empty();
        if (halt_processing)
            break;
        if (ask_level->second.getPrice() > bid_level->second.getPrice())
            break;
        Order &ask = ask_level->second.front();
        Order &bid = bid_level->second.front();
        matchOrders(ask, bid);
        processed_orders.push(bid);
        processed_orders.push(ask);
        if (bid.isFilled())
            deleteOrder(bid.getOrderID());
        if (ask.isFilled())
            deleteOrder(ask.getOrderID());
    }
    return !processed_orders.empty();
}

bool MapOrderBook::processMatching(Order &order, std::queue<Order> &processed_orders)
{
    if (order.isAsk())
    {
        auto level = bid_levels.rbegin();
        while (level != bid_levels.rend() && level->second.getPrice() >= order.getPrice())
        {
            Order &bid = level->second.front();
            matchOrders(order, bid);
            processed_orders.push(bid);
            processed_orders.push(order);
            if (bid.isFilled())
            {
                deleteOrder(bid.getOrderID());
                // Watch out for iterator invalidation. Deleting the order above could delete the level.
                level = bid_levels.rbegin();
            }
            if (order.isFilled())
                break;
        }
    }
    else
    {
        auto level = ask_levels.begin();
        while (level != ask_levels.end() && (level->second.getPrice() <= order.getPrice()))
        {
            Order &ask = level->second.front();
            matchOrders(ask, order);
            processed_orders.push(order);
            processed_orders.push(ask);
            if (ask.isFilled())
            {
                deleteOrder(ask.getOrderID());
                // Watch out for iterator invalidation. Deleting the order above could delete the level.
                level = ask_levels.begin();
            }
            if (order.isFilled())
                break;
        }
    }
    return !processed_orders.empty();
}

void MapOrderBook::matchOrders(Order &ask, Order &bid)
{
    uint64_t matched_quantity = std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(ask.isMarket() ? bid.getPrice() : ask.getPrice(), matched_quantity);
    ask.execute(bid.isMarket() ? ask.getPrice() : bid.getPrice(), matched_quantity);
}

bool MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        uint64_t executed_quantity = std::min(quantity, it->second.getOpenQuantity());
        it->second.execute(price, executed_quantity);
        return it->second.isFilled();
    }
    return false;
}

bool MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        uint64_t execute_quantity = std::min(quantity, it->second.getOpenQuantity());
        it->second.execute(it->second.getPrice(), execute_quantity);
        return it->second.isFilled();
    }
    return false;
}

bool MapOrderBook::canProcess(const Order &order) const
{
    if (orders.empty())
        return false;
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

bool MapOrderBook::cancelOrder(uint64_t order_id, uint64_t quantity)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        it->second.cancel(quantity);
        // Return true if cancelling the requested number of shares
        // reduces the orders open quantity to zero.
        return it->second.isFilled();
    }
    // Order is not in book - should never happen.
    return false;
}
