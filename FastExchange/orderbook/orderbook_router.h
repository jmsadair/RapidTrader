#ifndef FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#define FAST_EXCHANGE_ORDERBOOK_ROUTER_H
#include "vector_orderbook.h"

class OrderBookRouter {
public:
    // OrderBookRouter ADT is move only.
    OrderBookRouter(const OrderBookRouter&) = delete;
    OrderBookRouter& operator=(const OrderBookRouter&) = delete;

    /**
     * A constructor for the OrderBookRouter ADT.
     *
     * @param orderbook_sender_ the sender that each order book will
     *                           use to send message.
     */
    explicit OrderBookRouter(Messaging::Sender orderbook_sender_) :
            orderbook_sender(orderbook_sender_) {}

    /**
     * Processes a command for placing an order.
     *
     * @param command a command to place an order.
     */
    void processCommand(Message::Command::PlaceOrder& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.placeOrder(command);
    }

    /**
     * Processes a command for cancelling an order.
     *
     * @param command a command to cancel an order.
     */
    void processCommand(Message::Command::CancelOrder& command) {
        auto it = symbol_to_book.find(command.order_symbol);
        if (it != symbol_to_book.end())
            it->second.cancelOrder(command);
    }

    /**
     * Processes a command for adding creating a new order book.
     *
     * @param command a command to create a new order book.
     */
    void processCommand(Message::Command::AddOrderBook& command) {
        auto it = symbol_to_book.find(command.order_book_symbol);
        if (it == symbol_to_book.end())
            symbol_to_book.emplace(std::piecewise_construct, std::forward_as_tuple(command.order_book_symbol),
                                   std::forward_as_tuple(command.order_book_symbol, orderbook_sender));
    }

    /**
     * @return a sender that can be used to send messages to the order book router.
     */
    Messaging::Sender getSender() {
        return static_cast<Messaging::Sender>(receiver);
    }

    /**
     * Prepares the router to start receiving commands.
     */
    void start() {
        try {
            while (true) {
                receiver.wait()
                        .handle<Message::Command::PlaceOrder>([&](Message::Command::PlaceOrder &msg)
                        {
                            processCommand(msg);
                        })
                        .handle<Message::Command::AddOrderBook>([&](Message::Command::AddOrderBook &msg)
                        {
                            processCommand(msg);
                        })
                        .handle<Message::Command::CancelOrder>([&](Message::Command::CancelOrder &msg)
                        {
                           processCommand(msg);
                        });
            }
        } catch(const Messaging::CloseQueue&) {}
    }

private:
    // Maps a symbol to its corresponding order book.
    std::unordered_map<Symbol, OrderBook::VectorOrderBook> symbol_to_book;
    // Receives incoming command messages.
    Messaging::Receiver receiver;
    // Sends messages from each order book to the event handler.
    Messaging::Sender orderbook_sender;
};
#endif //FAST_EXCHANGE_ORDERBOOK_ROUTER_H
