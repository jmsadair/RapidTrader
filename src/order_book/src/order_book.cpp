#include "order_book.h"

namespace OrderBook {
    void OrderBook::execute(Order &order) {
        // Incoming order is an ask order.
        if (order.side == OrderSide::Ask) {
            auto price_level_it = bid_map.rbegin();
            while (price_level_it != bid_map.rend() && price_level_it->first >= order.price) {
                OrderList& order_list = price_level_it->second;
                while (!order_list.isEmpty() && order.status != OrderStatus::Filled) {
                    Order& other_order = order_list.front();
                    fillOrders(order, other_order);
                    if (other_order.status == OrderStatus::Filled) {
                        order_list.popFront();
                        removeOrder(other_order);
                    }
                }
                if (order_list.isEmpty())
                    bid_map.erase(order.price);
                if (order.status == OrderStatus::Filled)
                    return;
                ++price_level_it;
            }
            return;
        }
        // Incoming order is a bid order.
        auto price_level_it = ask_map.begin();
        while (price_level_it != ask_map.end() && price_level_it->first <= order.price) {
            OrderList& order_list = price_level_it->second;
            while (!order_list.isEmpty() && order.status != OrderStatus::Filled) {
                Order& other_order = order_list.front();
                fillOrders(order, other_order);
                if (other_order.status == OrderStatus::Filled) {
                    order_list.popFront();
                    removeOrder(other_order);
                }
            }
            if (order_list.isEmpty())
                ask_map.erase(order.price);
            if (order.status == OrderStatus::Filled)
                return;
            ++price_level_it;
        }
    }

    void OrderBook::fillOrders(Order& first_order, Order& second_order) {
        // Require that orders are not on same side.
        assert(first_order.side != second_order.side);
        if (first_order.quantity > second_order.quantity) {
            first_order.quantity -= second_order.quantity;
            second_order.quantity = 0;
            second_order.status = OrderStatus::Filled;
            first_order.status = OrderStatus::PartiallyFilled;
        } else if (first_order.quantity < second_order.quantity) {
            second_order.quantity -= first_order.quantity;
            first_order.quantity = 0;
            first_order.status = OrderStatus::Filled;
            second_order.status = OrderStatus::PartiallyFilled;
        } else {
            first_order.quantity = 0;
            second_order.quantity = 0;
            first_order.status = OrderStatus::Filled;
            second_order.status = OrderStatus::Filled;
        }
    }
}