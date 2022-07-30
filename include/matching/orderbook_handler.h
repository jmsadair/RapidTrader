#ifndef RAPID_TRADER_ORDERBOOK_HANDLER_H
#define RAPID_TRADER_ORDERBOOK_HANDLER_H
#include <memory>
#include "robin_hood.h"
#include "sender.h"
#include "map_orderbook.h"
#include "orderbook.h"

struct OrderBookHandler
{
    explicit OrderBookHandler(Concurrent::Messaging::Sender outgoing_messages_)
        : outgoing_messages(outgoing_messages_)
    {}

    void addOrderBook(uint32_t symbol_id)
    {
        if (id_to_book.find(symbol_id) == id_to_book.end())
            id_to_book.insert({symbol_id, std::make_unique<MapOrderBook>(symbol_id, outgoing_messages)});
    }

    void deleteOrderBook(uint32_t symbol_id)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
            id_to_book.erase(symbol_id);
    }

    void addOrder(const Order &order)
    {
        if (id_to_book.find(order.getSymbolID()) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(order.getSymbolID())->second.get();
            book->addOrder(order);
        }
    }

    void deleteOrder(uint32_t symbol_id, uint64_t order_id)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(symbol_id)->second.get();
            book->deleteOrder(order_id);
        }
    }

    void cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(symbol_id)->second.get();
            book->cancelOrder(order_id, cancelled_quantity);
        }
    }
    void replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint64_t new_price)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(symbol_id)->second.get();
            book->replaceOrder(order_id, new_order_id, new_price);
        }
    }

    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint64_t price)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(symbol_id)->second.get();
            book->executeOrder(order_id, quantity, price);
        }
    }
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
    {
        if (id_to_book.find(symbol_id) != id_to_book.end())
        {
            OrderBook *book = id_to_book.find(symbol_id)->second.get();
            book->executeOrder(order_id, quantity);
        }
    }

    std::string toString()
    {
        std::string book_handler_string;
        for (const auto &[symbol_id, book_ptr] : id_to_book)
        {
            if (book_ptr)
                book_handler_string += book_ptr->toString() + "\n";
        }
        return book_handler_string;
    }

    // Maps symbol IDs to order books.
    robin_hood::unordered_map<uint32_t, std::unique_ptr<OrderBook>> id_to_book;
    // Sends updates from market to event handler.
    Concurrent::Messaging::Sender outgoing_messages;
};
#endif // RAPID_TRADER_ORDERBOOK_HANDLER_H
