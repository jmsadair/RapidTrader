#include "order_book.h"

namespace OrderBook {
    void OrderBook::execute(Order &order) {
        // Incoming order is an ask order.
        bool is_ask = order.side == OrderSide::Ask;
        auto price_level_it = is_ask ? bid_map.begin() : ask_map.begin();
        auto last_it = is_ask ? bid_map.end() : ask_map.end();
        while (price_level_it != last_it && ((!is_ask && price_level_it->first <= order.price) ||
            (is_ask && price_level_it->first >= order.price))) {
            OrderList& order_list = price_level_it->second;
            while (!order_list.isEmpty() && order.status != OrderStatus::Filled) {
                Order& other_order = order_list.front();
                fillOrders(order, other_order);
                if (other_order.status == OrderStatus::Filled) {
                    order_map.erase(other_order.id);
                    order_list.popFront();
                }
            }
            if (order_list.isEmpty() && is_ask)
                bid_map.erase(price_level_it++);
            else if (order_list.isEmpty())
                ask_map.erase(price_level_it++);
            else
                ++price_level_it;
            if (order.status == OrderStatus::Filled)
                return;
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