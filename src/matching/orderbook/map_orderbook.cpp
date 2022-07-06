#include "map_orderbook.h"
#include "notification.h"
#include "log.h"

MapOrderBook::MapOrderBook(uint32_t symbol_id_, Messaging::Sender &outgoing_messages_)
    : symbol_id(symbol_id_), outgoing_messages(outgoing_messages_)
{}

void MapOrderBook::addLimitOrder(Order order)
{
    // Send notification that order was added.
    outgoing_messages.send(AddedOrder{order});
    // Match the order.
    processMatching(order);
    // If the order is filled and is not IOC or FOK, then the job is done.
    if (order.isFilled() && !order.isIoc() && !order.isFok())
        return;
    // If the order is not filled and is IOC or FOK, a notification that the order
    // was deleted must be sent.
    if (!order.isFilled() && (order.isIoc() || order.isFok())) {
        outgoing_messages.send(DeletedOrder{order});
    }
    // Add the order to the book.
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

void MapOrderBook::addMarketOrder(Order order) {
    // Send notification that order was added.
    outgoing_messages.send(AddedOrder{order});
    // Set the price of the order the max/min possible value for matching.
    order.setPrice(order.isAsk() ? 0 : std::numeric_limits<uint32_t>::max());
    if (order.isFok() && !canProcess(order))
        outgoing_messages.send(DeletedOrder{order});
    processMatching(order);
    if (!order.isFilled())
        outgoing_messages.send(DeletedOrder{order});
}

void MapOrderBook::deleteOrder(uint64_t order_id)
{
    auto it = orders.find(order_id);
    auto price_level_it = it->second.isAsk() ? ask_levels.find(it->second.getPrice()) : bid_levels.find(it->second.getPrice());
    price_level_it->second.deleteOrder(it->second);
    outgoing_messages.send(DeletedOrder{it->second});
    // Remove the price level if it is empty.
    if (price_level_it->second.empty())
        deleteLevel(it->second);
    orders.erase(it);
}

Level *MapOrderBook::addLevel(const Order &order)
{
    if (order.isAsk())
    {
        auto [it, success] = ask_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Ask, symbol_id));
        Level *level_ptr = &it->second;
        return level_ptr;
    }
    else
    {
        auto [it, success] = bid_levels.emplace(std::piecewise_construct, std::make_tuple(order.getPrice()),
            std::make_tuple(order.getPrice(), LevelSide::Bid, symbol_id));
        Level *level_ptr = &it->second;
        return level_ptr;
    }
}

void MapOrderBook::deleteLevel(const Order &order)
{
    if (order.isAsk())
        ask_levels.erase(order.getPrice());
    else
        bid_levels.erase(order.getPrice());
}

void MapOrderBook::processMatching()
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
        // Send order execution notifications for the matched orders.
        outgoing_messages.send(ExecutedOrder{bid});
        outgoing_messages.send(ExecutedOrder{ask});
        // Remove the orders from the book if they are filled.
        if (bid.isFilled())
            deleteOrder(bid.getOrderID());
        if (ask.isFilled())
            deleteOrder(ask.getOrderID());
    }
}

void MapOrderBook::processMatching(Order &order)
{
    // Order is on the ask side.
    if (order.isAsk())
    {
        // Maximum price bid level.
        auto level = bid_levels.rbegin();
        while (level != bid_levels.rend() && level->second.getPrice() >= order.getPrice())
        {
            Order &bid = level->second.front();
            matchOrders(order, bid);
            // Send order execution notifications for the matched orders.
            outgoing_messages.send(ExecutedOrder{bid});
            outgoing_messages.send(ExecutedOrder{order});
            // If the existing bid order is filled, remove it from the book.
            if (bid.isFilled())
            {
                deleteOrder(bid.getOrderID());
                level = bid_levels.rbegin();
            }
            // If the incoming ask order is filled, send order deletion notification
            // and exit the loop.
            if (order.isFilled())
            {
                outgoing_messages.send(DeletedOrder{order});
                break;
            }
        }
    }
    // Order is on the bid side.
    else
    {
        // Minimum price ask level.
        auto level = ask_levels.begin();
        while (level != ask_levels.end() && (level->second.getPrice() <= order.getPrice()))
        {
            Order &ask = level->second.front();
            matchOrders(ask, order);
            // Send order execution notifications for the matched orders.
            outgoing_messages.send(ExecutedOrder{order});
            outgoing_messages.send(ExecutedOrder{ask});
            // If the existing ask order is filled, remove it from the book.
            if (ask.isFilled())
            {
                deleteOrder(ask.getOrderID());
                level = ask_levels.begin();
            }
            // If the incoming bid order is filled, send order deletion notification
            // and exit the loop.
            if (order.isFilled())
            {
                outgoing_messages.send(DeletedOrder{order});
                break;
            }
        }
    }
}

void MapOrderBook::matchOrders(Order &ask, Order &bid)
{
    // Calculate the minimum quantity to match.
    uint64_t matched_quantity = std::min(ask.getOpenQuantity(), bid.getOpenQuantity());
    bid.execute(ask.isMarket() ? bid.getPrice() : ask.getPrice(), matched_quantity);
    ask.execute(bid.isMarket() ? ask.getPrice() : bid.getPrice(), matched_quantity);
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price)
{
    // Find the order.
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        // Calculate the minimum quantity to execute.
        uint64_t executed_quantity = std::min(quantity, it->second.getOpenQuantity());
        it->second.execute(price, executed_quantity);
        outgoing_messages.send(ExecutedOrder{it->second});
        if (it->second.isFilled())
            deleteOrder(order_id);
    }
}

void MapOrderBook::executeOrder(uint64_t order_id, uint64_t quantity)
{
    // Find the order.
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        // Calculate the minimum quantity to execute.
        uint64_t execute_quantity = std::min(quantity, it->second.getOpenQuantity());
        it->second.execute(it->second.getPrice(), execute_quantity);
        outgoing_messages.send(ExecutedOrder{it->second});
        if (it->second.isFilled())
            deleteOrder(order_id);
    }
}

bool MapOrderBook::canProcess(const Order &order) const
{
    // An order cannot be matched if there are no orders in the book.
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

void MapOrderBook::cancelOrder(uint64_t order_id, uint64_t quantity)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        it->second.cancel(quantity);
        outgoing_messages.send(UpdatedOrder{it->second});
        // Delete the order if cancelling the request quantity resulted in
        // it being filled.
        if (it->second.isFilled())
            deleteOrder(order_id);
    }
}
