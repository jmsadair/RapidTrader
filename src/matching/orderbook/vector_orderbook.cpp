#include <iostream>
#include "vector_orderbook.h"

// Only check the order book invariants in debug mode.
#ifndef NDEBUG
#    define ORDERBOOK_CHECK_INVARIANTS verifyOrderBookState()
#else
#    define ORDERBOOK_CHECK_INVARIANTS
#endif

void OrderBook::VectorOrderBook::placeOrder(Order &order)
{
    switch (order.type)
    {
    case OrderType::GoodTillCancel:
        placeGtcOrder(order);
        break;
    case OrderType::ImmediateOrCancel:
        placeIocOrder(order);
        break;
    case OrderType::FillOrKill:
        placeFokOrder(order);
        break;
    default:
        throw std::logic_error("Default case should never be reached!");
    }

    ORDERBOOK_CHECK_INVARIANTS;
}

void OrderBook::VectorOrderBook::reduceOrder(uint64_t order_id, uint64_t quantity_to_reduce_by)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        // Reducing the order quantity by the requested amount would mean that the order
        // has been fully executed. Remove the order from the book and notify the event
        // handler.
        if (it->second.quantity - quantity_to_reduce_by <= it->second.quantity_executed)
        {
            outgoing.send(Message::Event::OrderExecuted(it->second.user_id, it->second.id, it->second.price, it->second.quantity_executed));
            remove(it->second);
            return;
        }
        // Reduce the quantity by the desired amount;
        it->second.quantity -= quantity_to_reduce_by;
    }
}

void OrderBook::VectorOrderBook::placeGtcOrder(Order &order)
{
    match(order);
    if (!order.isFilled())
        insert(order);
}

void OrderBook::VectorOrderBook::placeFokOrder(Order &order)
{
    auto [price_chain, can_execute] = getPriceChain(order);
    // If the FOK order can be executed in full, then we match it with other orders. Otherwise,
    // the order is cancelled.
    if (can_execute)
        executePriceChain(price_chain, order);
    else
        outgoing.send(Message::Event::RejectionEvent(order.user_id, order.id, symbol_id, order.price, order.executableQuantity()));
}

void OrderBook::VectorOrderBook::placeIocOrder(Order &order)
{
    match(order);
    // IOC orders are cancelled if they are not fully executed.
    if (!order.isFilled())
        outgoing.send(Message::Event::RejectionEvent(order.user_id, order.id, symbol_id, order.price, order.executableQuantity()));
}

void OrderBook::VectorOrderBook::execute(Order &incoming, Order &existing)
{
    const uint64_t matched_quantity = std::min(incoming.executableQuantity(), existing.executableQuantity());
    const uint64_t existing_id = existing.id;
    const uint32_t existing_price = existing.price;
    const uint32_t incoming_price = incoming.isMarket() ? existing_price : incoming.price;
    incoming.quantity_executed += matched_quantity;
    existing.quantity_executed += matched_quantity;
    if (existing.isAsk())
        ask_price_levels[existing.price].volume -= matched_quantity;
    else
        bid_price_levels[existing.price].volume -= matched_quantity;
    // Existing order could not be completely filled.
    if (!existing.isFilled())
    {
        // Notify event handler that the existing order has been traded.
        outgoing.send(
            Message::Event::TradeEvent(existing.user_id, existing.id, incoming.id, existing.price, incoming_price, matched_quantity));
        // Notify that incoming order has been completely filled.
        if (incoming.isFilled())
        {
            outgoing.send(Message::Event::OrderExecuted(incoming.user_id, incoming.id, incoming_price, incoming.quantity));
        }
    // Existing order could be completely filled.
    }
    else
    {
        outgoing.send(Message::Event::OrderExecuted(existing.user_id, existing.id, existing.price, existing.quantity));
        // Remove existing order from the order book.
        if (existing.isAsk())
            ask_price_levels[existing.price].orders.pop_front();
        else
            bid_price_levels[existing.price].orders.pop_front();
        orders.erase(existing_id);
        // Notify that incoming order has been completely filled.
        if (incoming.isFilled())
            outgoing.send(Message::Event::OrderExecuted(incoming.user_id, incoming.id, incoming_price, incoming.quantity));
        // Notify that incoming order has been traded.
        else
            outgoing.send(
                Message::Event::TradeEvent(incoming.user_id, incoming.id, existing_id, incoming_price, existing_price, matched_quantity));
    }
}

void OrderBook::VectorOrderBook::executePriceChain(const std::vector<uint32_t> &price_chain, Order &order)
{
    if (order.isAsk())
    {
        for (const auto &price : price_chain)
        {
            max_bid_price = price;
            auto &price_level = bid_price_levels[price];
            while (!price_level.orders.empty() && !order.isFilled())
                execute(order, price_level.orders.front());
            max_bid_price -= price_level.orders.empty();
        }
    }
    else
    {
        for (const auto &price : price_chain)
        {
            min_ask_price = price;
            auto &price_level = ask_price_levels[price];
            while (!price_level.orders.empty() && !order.isFilled())
                execute(order, price_level.orders.front());
            if (price_level.orders.empty() && min_ask_price + 1 < ask_price_levels.size())
                ++min_ask_price;
        }
    }
}

std::pair<std::vector<uint32_t>, bool> OrderBook::VectorOrderBook::getPriceChain(const Order &order)
{
    std::vector<uint32_t> price_chain;
    uint64_t quantity_can_fill = 0;
    if (order.isAsk())
    {
        uint32_t price = max_bid_price;
        // The lowest bid price that the order can match with. If is a limit order, it is
        // the provided price of the order. If it is a market order, there is no minimum.
        const uint32_t minimum_price = order.action == OrderAction::Limit ? order.price : 1;
        while (price >= minimum_price && quantity_can_fill < order.quantity && price > 0)
        {
            quantity_can_fill += bid_price_levels[price].volume;
            if (!bid_price_levels[price].orders.empty())
                price_chain.push_back(price);
            --price;
        }
    }
    else
    {
        uint32_t price = min_ask_price;
        // The highest ask price that the order can match with. If is a limit order, it is
        // the provided price of the order. If it is a market order, there is no maximum.
        const uint32_t maximum_price = order.action == OrderAction::Limit ? order.price : std::numeric_limits<uint32_t>::max();
        while (price <= maximum_price && quantity_can_fill < order.quantity && price < ask_price_levels.size())
        {
            quantity_can_fill += ask_price_levels[price].volume;
            if (!ask_price_levels[price].orders.empty())
                price_chain.push_back(price);
            ++price;
        }
    }
    // If quantity_can_fill < order.quantity, it is not possible to fill the order.
    return std::make_pair(price_chain, quantity_can_fill >= order.quantity);
}

void OrderBook::VectorOrderBook::match(Order &order)
{
    if (order.isAsk())
    {
        auto price_level_it = bid_price_levels.begin() + static_cast<std::vector<PriceLevel>::difference_type>(max_bid_price);
        while (max_bid_price >= order.price && !order.isFilled())
        {
            while (!price_level_it->orders.empty() && !order.isFilled())
            {
                execute(order, price_level_it->orders.front());
            }
            // Quit if there are no orders left.
            if (orders.empty())
            {
                min_ask_price = std::numeric_limits<uint32_t>::max();
                max_bid_price = 0;
                return;
            }
            // Loop until the next non-empty price level is reached, decrementing the maximum bidding price as we go.
            while (price_level_it != bid_price_levels.begin() && price_level_it->orders.empty())
            {
                --max_bid_price;
                --price_level_it;
            }
            if (price_level_it == bid_price_levels.begin())
                return;
        }
    }
    else
    {
        auto price_level_it = ask_price_levels.begin() + static_cast<std::vector<PriceLevel>::difference_type>(min_ask_price);
        while (min_ask_price <= order.price && !order.isFilled())
        {
            while (!price_level_it->orders.empty() && !order.isFilled())
            {
                execute(order, price_level_it->orders.front());
            }
            // Quit if there are no orders left.
            if (orders.empty())
            {
                min_ask_price = std::numeric_limits<uint32_t>::max();
                max_bid_price = 0;
                return;
            }
            // Loop until the next non-empty price level is reached, incrementing the minimum asking price as we go.
            while (price_level_it != ask_price_levels.end() && price_level_it->orders.empty())
            {
                ++price_level_it;
                if (min_ask_price + 1 < ask_price_levels.size())
                    ++min_ask_price;
            }
            if (price_level_it == ask_price_levels.end())
                return;
        }
    }
}

void OrderBook::VectorOrderBook::insert(const Order &order)
{
    auto [it, success] = orders.insert({order.id, order});
    if (order.isAsk())
    {
        min_ask_price = std::min(min_ask_price, order.price);
        if (order.price >= ask_price_levels.size())
            ask_price_levels.resize(order.price + 1);
        ask_price_levels[order.price].orders.push_back(it->second);
        // Update the total volume of the price level with the executable quantity of the order.
        ask_price_levels[order.price].volume += order.executableQuantity();
    }
    else
    {
        max_bid_price = std::max(max_bid_price, order.price);
        if (order.price >= bid_price_levels.size())
            bid_price_levels.resize(order.price + 1);
        bid_price_levels[order.price].orders.push_back(it->second);
        // Update the total volume of the price level with the executable quantity of the order.
        bid_price_levels[order.price].volume += order.executableQuantity();
    }
}

void OrderBook::VectorOrderBook::remove(const Order &order)
{
    if (order.side == OrderSide::Ask)
    {
        auto orders_at_price_level = ask_price_levels.begin() + order.price;
        auto &order_list = orders_at_price_level->orders;
        auto price_level_it = ask_price_levels.begin() + min_ask_price;
        orders_at_price_level->volume -= order.executableQuantity();
        order_list.erase(order_list.iterator_to(order));
        orders.erase(order.id);
        // If the orderbook is empty, reset the min ask and max bid prices.
        if (orders.empty())
        {
            min_ask_price = std::numeric_limits<uint32_t>::max();
            max_bid_price = 0;
            return;
        }
        // If the price level of the order is now empty, increment minimum asking price until
        // the next non-empty price level is found.
        while (price_level_it->orders.empty() && price_level_it != ask_price_levels.end())
        {
            ++min_ask_price;
            ++price_level_it;
        }
    }
    else
    {
        auto orders_at_price_level = bid_price_levels.begin() + order.price;
        auto &order_list = orders_at_price_level->orders;
        auto price_level_it = bid_price_levels.begin() + max_bid_price;
        orders_at_price_level->volume -= order.executableQuantity();
        order_list.erase(order_list.iterator_to(order));
        orders.erase(order.id);
        // If the orderbook is empty, reset the min ask and max bid prices.
        if (orders.empty())
        {
            min_ask_price = std::numeric_limits<uint32_t>::max();
            max_bid_price = 0;
            return;
        }
        // If the price level of the order is now empty, decrement max bid price until
        // the next non-empty price level is found.
        while (price_level_it->orders.empty() && price_level_it != bid_price_levels.begin())
        {
            --max_bid_price;
            --price_level_it;
        }
    }
}

void OrderBook::VectorOrderBook::cancelOrder(uint64_t order_id)
{
    auto it = orders.find(order_id);
    if (it != orders.end())
    {
        remove(it->second);
        outgoing.send(Message::Event::RejectionEvent(
            it->second.user_id, it->second.id, symbol_id, it->second.price, it->second.executableQuantity()));
    }
    ORDERBOOK_CHECK_INVARIANTS;
}

void OrderBook::VectorOrderBook::verifyOrderBookState() const
{
    // Check that there are not any ask orders with price lower than
    // the minimum asking price. Check that each order list contains
    // only ask GTC orders that all have the same price, and that the price
    // level has the correct volume.
    for (uint64_t price = 0; price < ask_price_levels.size(); ++price)
    {
        verifyPriceLevelState(ask_price_levels[price], price, OrderSide::Ask);
    }
    // Check that there are not any bid orders with price greater than
    // the maximum bidding price. Check that each order list contains
    // only bid GTC orders that all have the same price, and that the price
    // level has the correct volume.
    for (uint64_t price = 0; price < bid_price_levels.size(); ++price)
    {
        verifyPriceLevelState(bid_price_levels[price], price, OrderSide::Bid);
    }
    // Check that all orders in the orderbook are in either ask_price_levels
    // or bid_price_levels.
    for (const auto &it : orders)
    {
        if (it.second.isAsk())
            assert(ask_price_levels[it.second.price].hasOrder(it.second));
        else
            assert(bid_price_levels[it.second.price].hasOrder(it.second));
    }
}

void OrderBook::VectorOrderBook::verifyPriceLevelState(
    const PriceLevel &price_level, uint32_t expected_price, OrderSide expected_order_side) const
{
    if (expected_order_side == OrderSide::Ask && expected_price < min_ask_price)
        assert(price_level.orders.empty());
    if (expected_order_side == OrderSide::Bid && expected_price > max_bid_price)
        assert(price_level.orders.empty());
    uint64_t quantity_sum = 0;
    for (const auto &order : price_level.orders)
    {
        // Check that all orders have the expected price.
        assert(order.price == expected_price);
        // Check that all orders are GTC (should not be any FOK or IOC orders).
        assert(order.type == OrderType::GoodTillCancel);
        // Check that all orders are on the expected side.
        assert(order.side == expected_order_side);
        // Sum the order quantities.
        quantity_sum += order.executableQuantity();
    }
    // Check that the sum of order quantities is the same as the price level volume.
    assert(quantity_sum == price_level.volume);
}