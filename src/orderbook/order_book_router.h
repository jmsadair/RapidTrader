#ifndef FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
#define FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
#include "vector_orderbook.h"
#include "command.h"

class OrderBookRouter {
public:
    // OrderBookRouter ADT is move only.
    OrderBookRouter(const OrderBookRouter&) = delete;
    OrderBookRouter& operator=(const OrderBookRouter&) = delete;

    /**
     * A constructor for the OrderBookRouter ADT.
     *
     * @param order_book_sender_ the sender that each order book will
     *                           use to send messages.
     */
    explicit OrderBookRouter(Messaging::Sender order_book_sender_) :
        order_book_sender(order_book_sender_)
    {}

    /**
     * Processes a command for placing an order.
     *
     * @param command a command to place an order.
     */
    inline void processCommand(Message::Command::PlaceOrder& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.placeOrder(command);
    };

    /**
     * Processes a command for cancelling an order.
     *
     * @param command a command to cancel an order.
     */
    inline void processCommand(Message::Command::CancelOrder& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.cancelOrder(command);
    };

    /**
     * Processes a command for adding creating a new order book.
     *
     * @param command a command to create a new order book.
     */
    inline void processCommand(Message::Command::AddOrderBook& command) {
        auto it = symbol_to_book.find(command.order_book_symbol);
        if (it == symbol_to_book.end())
            symbol_to_book.emplace(std::piecewise_construct, std::forward_as_tuple(command.order_book_symbol),
                                   std::forward_as_tuple(command.order_book_symbol, order_book_sender));
    }
private:
    // Maps a symbol to its corresponding order book.
    std::unordered_map<Symbol, OrderBook::VectorOrderBook> symbol_to_book;
    // A sender that each order book will use.
    Messaging::Sender order_book_sender;
};
#endif //FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
