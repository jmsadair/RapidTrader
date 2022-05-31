#ifndef FAST_EXCHANGE_ORDER_BOOK_H
#define FAST_EXCHANGE_ORDER_BOOK_H
#include <map>
#include <string>
#include <exception>
#include "order_list.h"
#include "commands.h"
#include "receiver.h"

namespace OrderBook {
    using Symbol = std::string;
    class OrderBook {
    public:
        /**
         * A constructor for the OrderBook ADT.
         *
         * @param order_book_symbol the symbol associated with the order book, require that
         *                          order_book_symbol is an arrangement of characters that represents
         *                          a publicly traded security.
         */
        explicit OrderBook(Symbol order_book_symbol) : symbol(std::move(order_book_symbol)) {}

        /**
         * Places a new order.
         *
         * @param order the order to place.
         */
        inline void placeOrder(PlaceOrderCommand& command) {
            switch(command.order_type) {
                case OrderType::GoodTillCancel:
                    handleGtcOrder(command);
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
         * @param command a command to cancel the order.
         */
        inline void cancelOrder(CancelOrderCommand& command) {
            auto it = order_map.find(command.order_id);
            if (it != order_map.end())
                removeOrder(it->second);
        }

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
         * Handles a command to place a GTC order. Mutates the command.
         *
         * @param command a command to place a GTC order.
         */
        void handleGtcOrder(PlaceOrderCommand& command) {
            execute(command);
            if (command.order_quantity) {
                Order order(command.order_action, command.order_side, command.order_type,
                            command.order_quantity,command.order_price, command.order_id,
                            command.user_id);
                insertOrder(order);
            }
        }

        /**
         * Attempts to execute an order as fully as possible. Mutates
         * the command.
         *
         * @param order a command to place an order.
         */
        void execute(PlaceOrderCommand& command) {
            bool is_ask = command.order_side == OrderSide::Ask;
            auto price_level_it = is_ask ? bid_map.begin() : ask_map.begin();
            auto last_it = is_ask ? bid_map.end() : ask_map.end();
            const auto can_match = is_ask ?
                                   [](Price order_price, Price price_level) { return price_level >= order_price; } :
                                   [](Price order_price, Price price_level) { return price_level <= order_price; };
            while (price_level_it != last_it && can_match(command.order_price, price_level_it->first)) {
                OrderList& order_list = price_level_it->second;
                while (!order_list.isEmpty() && command.order_quantity != 0) {
                    // Fill the order.
                    Order& order = order_list.front();
                    Quantity quantity_filled = std::min(command.order_quantity, order.quantity);
                    command.order_quantity -= quantity_filled;
                    order.quantity -= quantity_filled;
                    order.status = order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
                    if (order.status == OrderStatus::Filled) {
                        outgoing.send(OrderExecuted(order.user_id, order.id));
                        order_map.erase(order.id);
                        order_list.popFront();
                    }
                }
                if (order_list.isEmpty())
                    is_ask ? bid_map.erase(price_level_it++) : ask_map.erase(price_level_it++);
                else
                    ++price_level_it;
                if (!command.order_quantity) {
                    outgoing.send(OrderExecuted(command.user_id, command.order_id));
                    return;
                }
            }
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

        /**
         * Indicates whether the provided order is present in the
         * order book.
         *
         * @param order an order.
         * @return true if the order is in the order book and false
         *         otherwise.
         */
        inline bool hasOrder(const Order& order) { return order_map.find(order.id) != order_map.end(); }

        std::map<Price, OrderList, std::greater<>> bid_map;
        std::map<Price, OrderList, std::less<>> ask_map;
        std::unordered_map<OrderID, Order> order_map;
        Symbol symbol;
        Messaging::Sender outgoing;
    };
}
#endif //FAST_EXCHANGE_ORDER_BOOK_H
