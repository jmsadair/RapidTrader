#include <iostream>
#include "vector_orderbook.h"

void OrderBook::VectorOrderBook::placeOrder(Order order) {
    switch(order.type) {
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
}

void OrderBook::VectorOrderBook::placeGtcOrder(Order order) {
    match(order);
    if (!order.isFilled())
        insert(order);
}

void OrderBook::VectorOrderBook::placeFokOrder(Order order) {
    auto [price_chain, can_execute] = getPriceChain(order);
    if (can_execute)
        executePriceChain(price_chain, order);
    else
        outgoing.send(Message::Event::RejectionEvent(order.user_id, order.id, symbol_id, order.price,
                                                     order.executableQuantity()));
}

void OrderBook::VectorOrderBook::placeIocOrder(Order order) {
    match(order);
    // In the case of FOK order, if it is not fully filled, it will not be inserted into the orderbook.
    // It is instead cancelled.
    if (!order.isFilled())
        outgoing.send(Message::Event::RejectionEvent(order.user_id, order.id, symbol_id, order.price,
                                                     order.executableQuantity()));
}

void OrderBook::VectorOrderBook::execute(Order &incoming, Order &existing) {
    const uint64_t matched_quantity = std::min(incoming.executableQuantity(), existing.executableQuantity());
    const uint64_t existing_id = existing.id;
    const uint32_t existing_price  = existing.price;
    incoming.quantity_executed += matched_quantity;
    existing.quantity_executed += matched_quantity;
    if (existing.isAsk())
        ask_price_levels[existing.price].volume -= matched_quantity;
    else
        bid_price_levels[existing.price].volume -= matched_quantity;
    // Existing order could not be completely filled.
    if (!existing.isFilled()) {
        // Notify event handler that the existing order has been traded.
        outgoing.send(Message::Event::TradeEvent(existing.user_id, existing.id, incoming.id, existing.price,
                                                 incoming.price, matched_quantity));
        // Notify that incoming order has been completely filled.
        if (incoming.isFilled())
            outgoing.send(Message::Event::OrderExecuted(incoming.user_id, incoming.id, incoming.price,
                                                        incoming.quantity));
    // Existing order could be completely filled.
    } else {
        outgoing.send(Message::Event::OrderExecuted(existing.user_id, existing.id, existing.price, existing.quantity));
        // Remove existing order from the order book.
        if (existing.isAsk())
            ask_price_levels[existing.price].order_list.pop_front();
        else
            bid_price_levels[existing.price].order_list.pop_front();
        orders.erase(existing_id);
        // Notify that incoming order has been completely filled.
        if (incoming.isFilled())
            outgoing.send(Message::Event::OrderExecuted(incoming.user_id, incoming.id, incoming.price,
                                                        incoming.quantity));
        // Notify that incoming order has been traded.
        else
            outgoing.send(Message::Event::TradeEvent(incoming.user_id, incoming.id, existing_id, incoming.price,
                                                     existing_price, matched_quantity));
    }

}

void OrderBook::VectorOrderBook::executePriceChain(const std::vector<uint32_t> &price_chain, Order &order) {
    if (order.isAsk()) {
        for (const auto& price : price_chain) {
            max_bid_price = price;
            auto& price_level = bid_price_levels[price];
            while (!price_level.order_list.empty() && !order.isFilled())
                execute(order, price_level.order_list.front());
            max_bid_price -= price_level.order_list.empty();
        }
    } else {
        for (const auto& price : price_chain) {
            min_ask_price = price;
            auto& price_level = ask_price_levels[price];
            while (!price_level.order_list.empty() && !order.isFilled())
                execute(order, price_level.order_list.front());
            min_ask_price += price_level.order_list.empty();
        }
    }
}

std::pair<std::vector<uint32_t>, bool> OrderBook::VectorOrderBook::getPriceChain(const Order &order) {
    std::vector<uint32_t> price_chain;
    uint64_t quantity_can_fill = 0;
    if (order.isAsk()) {
        uint32_t price = max_bid_price;
        while (price >= order.price && quantity_can_fill < order.quantity && price > 0) {
            quantity_can_fill += bid_price_levels[price].volume;
            if (!bid_price_levels[price].order_list.empty())
                price_chain.push_back(price);
            --price;
        }
    } else {
        uint32_t price = min_ask_price;
        while (price <= order.price && quantity_can_fill < order.quantity && price < ask_price_levels.size()) {
            quantity_can_fill += ask_price_levels[price].volume;
            if (!ask_price_levels[price].order_list.empty())
                price_chain.push_back(price);
            ++price;
        }
    }
    return std::make_pair(price_chain, quantity_can_fill >= order.quantity);
}

void OrderBook::VectorOrderBook::match(Order &order) {
    if (order.isAsk()) {
        auto price_level_it = bid_price_levels.begin() +
                static_cast<std::vector<PriceLevel>::difference_type>(max_bid_price);
        while (max_bid_price >= order.price) {
            auto& price_level = *price_level_it;
            while (!price_level.order_list.empty() && !order.isFilled()) {
                execute(order, price_level.order_list.front());
            }
            max_bid_price -= price_level.order_list.empty();
            --price_level_it;
        }
    } else {
        auto price_level_it = ask_price_levels.begin() +
                static_cast<std::vector<PriceLevel>::difference_type>(min_ask_price);
        while (min_ask_price <= order.price) {
            auto& price_level = *price_level_it;
            while (!price_level.order_list.empty() && !order.isFilled()) {
                execute(order, price_level.order_list.front());
            }
            min_ask_price += price_level.order_list.empty();
            ++price_level_it;
        }
    }
}

void OrderBook::VectorOrderBook::insert(Order order) {
    auto [it, success] = orders.insert({order.id, order});
    if (order.isAsk()) {
        min_ask_price = std::min(min_ask_price, order.price);
        if (order.price >= ask_price_levels.size())
            ask_price_levels.resize(order.price + 1);
        auto& price_level = *(ask_price_levels.begin() + order.price);
        price_level.order_list.push_back(it->second);
        // Update the total volume of the price level with the executable quantity of the order.
        price_level.volume += order.executableQuantity();
    } else {
        max_bid_price = std::max(max_bid_price, order.price);
        if (order.price >= bid_price_levels.size())
            bid_price_levels.resize(order.price + 1);
        auto& price_level = *(bid_price_levels.begin() + order.price);
        price_level.order_list.push_back(it->second);
        // Update the total volume of the price level with the executable quantity of the order.
        price_level.volume += order.executableQuantity();
    }
}

void OrderBook::VectorOrderBook::remove(Order &order) {
    if (order.side == OrderSide::Ask) {
        auto orders_at_price_level = ask_price_levels.begin() + order.price;
        auto& order_list = orders_at_price_level->order_list;
        auto price_level_it = ask_price_levels.begin() + min_ask_price;
        orders_at_price_level->volume -= order.quantity;
        order_list.erase(order_list.iterator_to(order));
        orders.erase(order.id);
        // If the orderbook is empty, reset the min ask and max bid prices.
        if (orders.empty()) {
            min_ask_price = std::numeric_limits<uint32_t>::max();
            max_bid_price = 0;
            return;
        }
        // If the price level of the order is now empty, increment minimum asking price until
        // the next non-empty price level is found.
        while (price_level_it->order_list.empty() && price_level_it != ask_price_levels.end()) {
            ++min_ask_price;
            ++price_level_it;
        }
    } else {
        auto orders_at_price_level = bid_price_levels.begin() + order.price;
        auto& order_list = orders_at_price_level->order_list;
        auto price_level_it = bid_price_levels.begin() + max_bid_price;
        orders_at_price_level->volume -= order.quantity;
        order_list.erase(order_list.iterator_to(order));
        orders.erase(order.id);
        // If the orderbook is empty, reset the min ask and max bid prices.
        if (orders.empty()) {
            min_ask_price = std::numeric_limits<uint32_t>::max();
            max_bid_price = 0;
            return;
        }
        // If the price level of the order is now empty, decrement max bid price until
        // the next non-empty price level is found.
        while (price_level_it->order_list.empty() && price_level_it != bid_price_levels.begin()) {
            --max_bid_price;
            --price_level_it;
        }
    }
}

void OrderBook::VectorOrderBook::cancelOrder(uint64_t order_id) {
    auto it = orders.find(order_id);
    if (it != orders.end()) {
        remove(it->second);
        outgoing.send(Message::Event::RejectionEvent(it->second.user_id, it->second.id, symbol_id, it->second.price,
                                                     it->second.executableQuantity()));
    }
}