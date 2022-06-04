#ifndef FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#define FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#include <vector>
#include <limits>
#include "robin_hood.h"
#include "price_level.h"
#include "event.h"
#include "receiver.h"
#include "orderbook_interface.h"

constexpr size_t DEFAULT_PRICE_LEVELS_SIZE = 1000000;

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
        inline void placeOrder(Message::Command::PlaceOrder &command) override  {
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
        inline void cancelOrder(Message::Command::CancelOrder &command) override {
            auto it = orders.find(command.order_id);
            if (it != orders.end()) {
                remove(it->second);
                outgoing.send(Message::Event::RejectionEvent(it->second.user_id, it->second.id, symbol, it->second.price,
                                                             it->second.quantity_to_fill));
            }
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
        inline void execute(Message::Command::PlaceOrder &command, Order &order) override {
            Quantity matched_quantity = std::min(command.order_quantity, order.quantity_to_fill);
            command.order_quantity -= matched_quantity;
            order.quantity_to_fill -= matched_quantity;
            if (order.quantity_to_fill != 0) {
                order.status = OrderStatus::PartiallyFilled;
                outgoing.send(Message::Event::TradeEvent(order.user_id, order.id, command.order_id, order.price,
                                                         command.order_price, matched_quantity));
            } else {
                order.status = OrderStatus::Filled;
                outgoing.send(Message::Event::OrderExecuted(order.user_id, order.id, order.price,
                                                            order.quantity));
            }
        }

        /**
         * @inheritdoc
         */
        void match(Message::Command::PlaceOrder &command)  override {
            if (command.order_side == OrderSide::Ask) {
                auto price_level_it = bid_price_levels.begin() + max_bid_price;
                while (max_bid_price >= command.order_price) {
                    auto& price_level = *price_level_it;
                    while (!price_level.order_list.empty() && command.order_quantity != 0) {
                        auto& order = price_level.order_list.front();
                        execute(command, order);
                        if (order.status == OrderStatus::Filled) {
                            const OrderID id_to_erase = order.id;
                            price_level.order_list.pop_front();
                            orders.erase(id_to_erase);
                        }
                    }
                    max_bid_price -= price_level.order_list.empty();
                    --price_level_it;
                }
            } else {
                auto price_level_it = ask_price_levels.begin() + min_ask_price;
                while (min_ask_price <= command.order_price) {
                    auto& price_level = *price_level_it;
                    while (!price_level.order_list.empty() && command.order_quantity != 0) {
                        auto& order = price_level.order_list.front();
                        execute(command, order);
                        if (order.status == OrderStatus::Filled) {
                            const OrderID id_to_erase = order.id;
                            price_level.order_list.pop_front();
                            orders.erase(id_to_erase);
                        }
                    }
                    min_ask_price += price_level.order_list.empty();
                    ++price_level_it;
                }
            }
        }

        /**
         * @inheritdoc
         */
        void handleGtcOrder(Message::Command::PlaceOrder &command) override {
            const Quantity initial_quantity = command.order_quantity;
            match(command);
            const Quantity remaining_quantity = command.order_quantity;
            if (remaining_quantity != 0) {
                insert(std::move(Order(command.order_action, command.order_side, command.order_type,
                                                     initial_quantity, remaining_quantity,
                                                     command.order_price, command.order_id,
                                                     command.user_id, remaining_quantity == initial_quantity ?
                                                     OrderStatus::Accepted : OrderStatus::PartiallyFilled)));
                return;
            }
            outgoing.send(Message::Event::OrderExecuted(command.user_id, command.order_id,
                                                        command.order_price, initial_quantity));
        }

        /**
         * @inheritdoc
         */
        inline void insert(Order order) override {
            auto [it, success] = orders.insert({order.id, order});
            if (order.side == OrderSide::Ask) {
                min_ask_price = std::min(min_ask_price, order.price);
                if (order.price >= ask_price_levels.size())
                    ask_price_levels.resize(order.price + 1);
                auto& price_level = *(ask_price_levels.begin() + order.price);
                price_level.order_list.push_back(it->second);
                price_level.volume += order.quantity_to_fill;
            } else {
                max_bid_price = std::max(max_bid_price, order.price);
                if (order.price >= bid_price_levels.size())
                    bid_price_levels.resize(order.price + 1);
                auto& price_level = *(bid_price_levels.begin() + order.price);
                price_level.order_list.push_back(it->second);
                price_level.volume += order.quantity_to_fill;
            }
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
        robin_hood::unordered_map<OrderID, Order> orders;
        std::vector<PriceLevel> ask_price_levels;
        std::vector<PriceLevel> bid_price_levels;
        Price min_ask_price = std::numeric_limits<Price>::max();
        Price max_bid_price = 0;
        std::string symbol;
        Messaging::Sender outgoing;
    };
}

#endif //FAST_EXCHANGE_VECTOR_ORDERBOOK_H
