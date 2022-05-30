#ifndef FAST_EXCHANGE_ORDER_BOOK_H
#define FAST_EXCHANGE_ORDER_BOOK_H
#include <map>
#include <string>
#include <exception>
#include "order_list.h"

namespace OrderBook {
    class OrderBook {
    public:
        /**
         * A constructor for the OrderBook ADT.
         *
         * @param order_book_symbol the symbol associated with the order book, require that
         *                          order_book_symbol is an arrangement of characters that represents
         *                          a publicly traded security.
         */
        explicit OrderBook(std::string order_book_symbol) : symbol(std::move(order_book_symbol)) {}

        /**
         * Places a new order.
         *
         * @param order the order to place.
         */
        inline void placeOrder(Order& order) {
            switch(order.type) {
                case OrderType::GoodTillCancel:
                    handleGtcOrder(order);
                    break;
                case OrderType::ImmediateOrCancel:
                    throw std::logic_error("IOC orders are not currently supported!");
                case OrderType::FillOrKill:
                    throw std::logic_error("FOK orders are not currently supported!");
                default:
                    throw std::logic_error("Default case should never be reached!");
            }
        }

        /**
         * Cancels the order if it exists in the order book.
         *
         * @param order the order to cancel.
         */
        inline void cancelOrder(Order& order) {
            removeOrder(order);
            order.status = OrderStatus::Cancelled;
        }

        /**
         * Indicates whether the provided order is present in the
         * order book.
         *
         * @param order an order.
         * @return true if the order is in the order book and false
         *         otherwise.
         */
        inline bool hasOrder(const Order& order) { return order_map.find(order.id) != order_map.end(); }

        /**
         * @return the string representation of the order book.
         */
        std::string toString() const {
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
        };

    private:
        /**
         * Attempts to execute an order as fully as possible.
         *
         * @param order the order to execute.
         */
        void execute(Order& order) {
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

        /**
         * Given a bid and an ask order at the same price level, fills
         * the orders as fully as possible. Mutates the orders by updating their
         * status and quantity.
         *
         * @param first_order an order.
         * @param second_order another order, require that second_order is an ask
         *                     order if first_order is a bid order; otherwise, if
         *                     first_order is an ask_order, require that second_order
         *                     is a bid order.
         */
        static inline void fillOrders(Order& first_order, Order& second_order) {
            // Require that orders are not on same side.
            assert(first_order.side != second_order.side);
            uint64_t quantity_filled = std::min(first_order.quantity, second_order.quantity);
            first_order.quantity -= quantity_filled;
            second_order.quantity -= quantity_filled;
            second_order.status = second_order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
            first_order.status = first_order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
        }

        /**
        * Places a GTC (Good 'Till Cancel) order.
        *
        * @param order a GTC order.
        */
        void handleGtcOrder(Order& order) {
            execute(order);
            if (order.status != OrderStatus::Filled)
                insertOrder(order);
        }

        /**
         * Removes the order from the order book, if its exists.
         *
         * @param order the order to remove.
         */
        inline void removeOrder(Order& order) {
            auto it = order.side == OrderSide::Ask ?
                    ask_map.find(order.price) : bid_map.find(order.price);
            it->second.removeOrder(order);
            if (it->second.isEmpty())
                order.side == OrderSide::Ask ? ask_map.erase(it) : bid_map.erase(it);
            order_map.erase(order.id);
        }

        /**
         * Inserts an order into the order book.
         *
         * @param order the order to insert.
         */
        inline void insertOrder(Order& order) {
            auto pair = order.side == OrderSide::Ask ?
                    ask_map.emplace(order.price, order) : bid_map.emplace(order.price, order);
            if (!pair.second) { pair.first->second.addOrder(order); }
            order_map.insert(std::make_pair(order.id, order));
        }

        std::map<Price, OrderList, std::greater<>> bid_map;
        std::map<Price, OrderList, std::less<>> ask_map;
        std::unordered_map<OrderID, Order> order_map;
        std::string symbol;
    };
}
#endif //FAST_EXCHANGE_ORDER_BOOK_H
