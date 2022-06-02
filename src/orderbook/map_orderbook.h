#ifndef FAST_EXCHANGE_MAP_ORDERBOOK_H
#define FAST_EXCHANGE_MAP_ORDERBOOK_H
#include <map>
#include "orderbook_interface.h"

namespace OrderBook {
    class MapOrderBook : public OrderBook {
    public:
        /**
         * A constructor for the MapOrderBook ADT.
         *
         * @param symbol_ the symbol associated with the order book, require that
         *                order_book_symbol is an arrangement of characters that represents
         *                a publicly traded security.
         * @param outgoing_
         */
        MapOrderBook(Symbol symbol_, Messaging::Sender outgoing_) :
                symbol(std::move(symbol_)), outgoing(outgoing_)
        {}

        /**
         * @inheritdoc
         */
        void placeOrder(PlaceOrderCommand &command) override {
            switch (command.order_type) {
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
         * @inheritdoc
         */
        inline void cancelOrder(CancelOrderCommand& command) override {
            auto it = orders.find(command.order_id);
            if (it != orders.end())
                remove(it->second);
        }

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline bool hasOrder(OrderID order_id) const override {
            return orders.find(order_id) != orders.end();
        }

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline const Order& getOrder(OrderID order_id) const override {
            return orders.at(order_id);
        }


        /**
         * @inheritdoc
         */
        [[nodiscard]] std::string toString() const override {
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
         * @inheritdoc
         */
        void handleGtcOrder(PlaceOrderCommand& command) override {
            const Quantity remaining_quantity = execute(command);
            if (remaining_quantity != 0) {
                auto& order = insert(std::move(Order(command.order_action, command.order_side, command.order_type,
                                      remaining_quantity,command.order_price, command.order_id,
                                      command.user_id, remaining_quantity == command.order_quantity ?
                                                       OrderStatus::Accepted : OrderStatus::PartiallyFilled)));
                outgoing.send(OrderAddedToBook(order.user_id, order.id, order.quantity, order.status));
            }
        }

        /**
         * @inheritdoc
         */
        Quantity execute(PlaceOrderCommand& command) override {
            Quantity incoming_order_quantity = command.order_quantity;
            bool is_ask = command.order_side == OrderSide::Ask;
            auto price_level_it = is_ask ? bid_map.begin() : ask_map.begin();
            auto last_it = is_ask ? bid_map.end() : ask_map.end();
            const auto can_match = is_ask ?
                                   [](Price order_price, Price price_level) { return price_level >= order_price; } :
                                   [](Price order_price, Price price_level) { return price_level <= order_price; };
            while (price_level_it != last_it && can_match(command.order_price, price_level_it->first)) {
                auto& order_list = price_level_it->second.order_list;
                while (!order_list.empty() && incoming_order_quantity != 0) {
                    // Fill the order.
                    Order& order = order_list.front();
                    Quantity quantity_filled = std::min(incoming_order_quantity, order.quantity);
                    incoming_order_quantity -= quantity_filled;
                    order.quantity -= quantity_filled;
                    order.status = order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
                    if (order.status == OrderStatus::Filled) {
                        outgoing.send(OrderExecuted(order.user_id, order.id));
                        // Note: Be careful! The order must be removed from the order list before
                        // it can be removed from the order book.
                        order_list.pop_front();
                        orders.erase(order.id);
                    }
                }
                if (order_list.empty())
                    is_ask ? bid_map.erase(price_level_it++) : ask_map.erase(price_level_it++);
                else
                    ++price_level_it;
                if (incoming_order_quantity == 0) {
                    outgoing.send(OrderExecuted(command.user_id, command.order_id));
                    return incoming_order_quantity;
                }
            }
            return incoming_order_quantity;
        }

        /**
         * @inheritdoc
         */
        inline void remove(Order& order) override {
            auto it = order.side == OrderSide::Ask ? ask_map.find(order.price) : bid_map.find(order.price);
            it->second.removeOrder(order);
            if (it->second.order_list.empty())
                order.side == OrderSide::Ask ? ask_map.erase(it) : bid_map.erase(it);
            orders.erase(order.id);
        }

        /**
         * @inheritdoc
         */
        inline const Order& insert(Order order) override {
            auto id_order_pair = orders.insert(std::make_pair(order.id, order));
            auto price_list_pair = order.side == OrderSide::Ask ?
                    ask_map.emplace(id_order_pair.first->second.price, id_order_pair.first->second) :
                    bid_map.emplace(id_order_pair.first->second.price, id_order_pair.first->second);
            if (!price_list_pair.second)
                price_list_pair.first->second.order_list.push_back(id_order_pair.first->second);
            return id_order_pair.first->second;
        }

        std::unordered_map<OrderID, Order> orders;
        std::map<Price, PriceLevel, std::greater<>> bid_map;
        std::map<Price, PriceLevel, std::less<>> ask_map;
        Messaging::Sender outgoing;
        Symbol symbol;
    };
}
#endif //FAST_EXCHANGE_MAP_ORDERBOOK_H
