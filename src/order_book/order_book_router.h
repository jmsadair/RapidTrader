#ifndef FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
#define FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
#include "order_book.h"
#include "commands.h"

class OrderBookRouter {
public:
    // OrderBookRouter ADT is move only.
    OrderBookRouter(const OrderBookRouter&) = delete;
    OrderBookRouter& operator=(const OrderBookRouter&) = delete;

    /**
     * Processes a command for placing an order.
     *
     * @param command a command to place an order.
     */
    inline void processCommand(PlaceOrderCommand& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.placeOrder(command);
    };

    /**
     * Processes a command for cancelling an order.
     *
     * @param command a command to cancel an order.
     */
    inline void processCommand(CancelOrderCommand& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.cancelOrder(command);
    };

    /**
     * Processes a command for adding creating a new order book.
     *
     * @param command a command to create a new order book.
     */
    inline void processCommand(AddOrderBookCommand& command) {
        auto it = symbol_to_book.find(command.order_book_symbol);
        if (it == symbol_to_book.end())
            symbol_to_book.emplace(command.order_book_symbol, command.order_book_symbol);
    }
private:
    // Maps a symbol to its corresponding order book.
    std::unordered_map<Symbol, OrderBook::OrderBook> symbol_to_book;
};
#endif //FAST_EXCHANGE_ORDER_BOOK_ROUTER_H
