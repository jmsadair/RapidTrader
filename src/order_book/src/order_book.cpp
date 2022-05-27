#include "order_book.h"

namespace OrderBook {
    void OrderBook::execute(Order &order) {
        bool is_ask = order.side == OrderSide::Ask;
        auto price_level_it = is_ask ? bid_map.begin() : ask_map.begin();
        auto last_it = is_ask ? bid_map.end() : ask_map.end();
        const auto can_match = is_ask ?
                [](Price order_price, Price price_level) { return price_level >= order_price; } :
                [](Price order_price, Price price_level) { return price_level <= order_price; };
        while (price_level_it != last_it && can_match(order.price, price_level_it->first)) {
            OrderList& order_list = price_level_it->second;
            while (!order_list.isEmpty() && order.status != OrderStatus::Filled) {
                Order& other_order = order_list.front();
                fillOrders(order, other_order);
                if (other_order.status == OrderStatus::Filled) {
                    order_map.erase(other_order.id);
                    order_list.popFront();
                }
            }
            if (order_list.isEmpty())
                is_ask ? bid_map.erase(price_level_it++) : ask_map.erase(price_level_it++);
            else
                ++price_level_it;
            if (order.status == OrderStatus::Filled)
                return;
        }
    }

    std::string OrderBook::toString() const {
        std::string order_book_string = "----------ASK SIDE ORDERS----------\n\n";
        for (const auto& [price_level, order_list] : ask_map) {
            order_book_string += "Price Level: " + std::to_string(price_level) + "\n" + order_list.toString();
        }
        order_book_string += "-----------------------------------\n\n----------BID SIDE ORDERS----------\n\n";
        for (const auto& [price_level, order_list] : bid_map) {
            order_book_string += "Price Level: " + std::to_string(price_level) + "\n" + order_list.toString();
        }
        order_book_string += "-----------------------------------\n\n";
        return order_book_string;
    }
}