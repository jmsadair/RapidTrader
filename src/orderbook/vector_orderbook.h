#ifndef FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#define FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#include <vector>
#include "orderbook_interface.h"

constexpr size_t DEFAULT_PRICE_LEVELS_SIZE = 100000;

namespace OrderBook {
    class VectorOrderBook : public OrderBook {
    public:
        /**
         * A constructor for the VectorOrderBook ADT.
         *
         * @param symbol_ the symbol associated with the order book.
         * @param outgoing_ a message sender used to communicate with users.
         */
        VectorOrderBook(Symbol symbol_, Messaging::Sender outgoing_) :
            symbol(std::move(symbol_)), outgoing(outgoing_)
        {
            ask_price_levels.resize(DEFAULT_PRICE_LEVELS_SIZE);
            bid_price_levels.resize(DEFAULT_PRICE_LEVELS_SIZE);
        }

       /**
        * @inheritdoc
        */
        inline void placeOrder(PlaceOrderCommand& command) override  {
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
        std::string toString() const override {
            std::string order_book_string = "----------ASK SIDE ORDERS----------\n\n";
            for (Price price = min_ask_price; price < ask_price_levels.size(); ++price) {
                if (!ask_price_levels[price].order_list.empty())
                    order_book_string += "Price Level: " + std::to_string(price) + "\n" +
                            ask_price_levels[price].toString();
            }
            order_book_string += "-----------------------------------\n\n----------BID SIDE ORDERS----------\n\n";
            for (Price price = max_bid_price; price > 0; --price) {
                if (!bid_price_levels[price].order_list.empty())
                    order_book_string += "Price Level: " + std::to_string(price) + "\n" +
                            bid_price_levels[price].toString();
            }
            order_book_string += "-----------------------------------\n\n";
            return order_book_string;
        }

    private:
        /**
         * @inheritdoc
         */
        Quantity execute(PlaceOrderCommand& command)  override {
            Quantity incoming_order_quantity = command.order_quantity;
            if (command.order_side == OrderSide::Ask) {
                auto price_level_it = bid_price_levels.begin() + max_bid_price;
                while (price_level_it != bid_price_levels.begin() && max_bid_price >= command.order_price) {
                    auto& order_list = price_level_it->order_list;
                    while (!order_list.empty() && incoming_order_quantity != 0) {
                        Order& order = order_list.front();
                        Quantity quantity_filled = std::min(incoming_order_quantity, order.quantity);
                        incoming_order_quantity -= quantity_filled;
                        order.quantity -= quantity_filled;
                        (*price_level_it).volume -= quantity_filled;
                        order.status = order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
                        if (order.status == OrderStatus::Filled) {
                            outgoing.send(OrderExecuted(order.user_id, order.id));
                            order_list.pop_front();
                            orders.erase(order.id);
                        }
                    }
                    if (order_list.empty())
                        --max_bid_price;
                    --price_level_it;
                    if (incoming_order_quantity == 0) {
                        outgoing.send(OrderExecuted(command.user_id, command.order_id));
                        return incoming_order_quantity;
                    }
                }
            } else {
                auto price_level_it = ask_price_levels.begin() + min_ask_price;
                while (price_level_it != bid_price_levels.begin() && min_ask_price <= command.order_price) {
                    auto& order_list = price_level_it->order_list;
                    while (!order_list.empty() && incoming_order_quantity != 0) {
                        Order& order = order_list.front();
                        Quantity quantity_filled = std::min(incoming_order_quantity, order.quantity);
                        incoming_order_quantity -= quantity_filled;
                        order.quantity -= quantity_filled;
                        (*price_level_it).volume -= quantity_filled;
                        order.status = order.quantity != 0 ? OrderStatus::PartiallyFilled : OrderStatus::Filled;
                        if (order.status == OrderStatus::Filled) {
                            outgoing.send(OrderExecuted(order.user_id, order.id));
                            order_list.pop_front();
                            orders.erase(order.id);
                        }
                    }
                    if (order_list.empty())
                        ++min_ask_price;
                    ++price_level_it;
                    if (incoming_order_quantity == 0) {
                        outgoing.send(OrderExecuted(command.user_id, command.order_id));
                        return incoming_order_quantity;
                    }
                }
            }
            return incoming_order_quantity;
        }

        /**
         * @inheritdoc
         */
        void handleGtcOrder(PlaceOrderCommand& command) override {
            const Quantity remaining_quantity = execute(command);
            if (remaining_quantity != 0) {
                auto& order = insert(std::move(Order(command.order_action, command.order_side, command.order_type,
                                                     remaining_quantity, command.order_price, command.order_id,
                                                     command.user_id, remaining_quantity == command.order_quantity ?
                                                     OrderStatus::Accepted : OrderStatus::PartiallyFilled)));
                outgoing.send(OrderAddedToBook(order.user_id, order.id, order.quantity, order.status));
            }
        }

        /**
         * @inheritdoc
         */
        inline const Order& insert(Order order) override {
            auto [it, success] = orders.try_emplace(order.id, order);
            auto& order_side_levels = order.side == OrderSide::Ask ? ask_price_levels : bid_price_levels;
            if (order.price >= order_side_levels.size())
                order_side_levels.resize(order.price + 1);
            order_side_levels[order.price].order_list.push_back(it->second);
            return it->second;
        }

        /**
         * @inheritdoc
         */
        inline void remove(Order& order) override {
            if (order.side == OrderSide::Ask) {
                auto orders_at_price_level = ask_price_levels.begin() + order.price;
                auto& order_list = orders_at_price_level->order_list;
                auto price_level_it = ask_price_levels.begin() + min_ask_price;
                orders_at_price_level->volume -= order.quantity;
                order_list.erase(order_list.iterator_to(order));
                orders.erase(order.id);
                if (orders.empty())
                    return;
                while (price_level_it->order_list.empty() && price_level_it != ask_price_levels.end()) {
                    ++min_ask_price;
                    ++price_level_it;
                }
            } else {
                auto orders_at_price_level = bid_price_levels.begin() + order.price;
                auto& order_list = orders_at_price_level->order_list;
                auto price_level_it = ask_price_levels.begin() + max_bid_price;
                orders_at_price_level->volume -= order.quantity;
                order_list.erase(order_list.iterator_to(order));
                orders.erase(order.id);
                while (price_level_it->order_list.empty() && price_level_it != bid_price_levels.begin()) {
                    --max_bid_price;
                    --price_level_it;
                }
            }
        }

        // IMPORTANT: Note that the declaration of orders MUST be
        // declared before the declaration of the price level vectors.
        // Class members are destroyed in the reverse order of their declaration,
        // so the price level vectors will be destroyed before orders. This is
        // required for the intrusive list.
        std::unordered_map<OrderID, Order> orders;
        std::vector<PriceLevel> ask_price_levels;
        std::vector<PriceLevel> bid_price_levels;
        Price min_ask_price = 1;
        Price max_bid_price = 1;
        std::string symbol;
        Messaging::Sender outgoing;
    };
}

#endif //FAST_EXCHANGE_VECTOR_ORDERBOOK_H
