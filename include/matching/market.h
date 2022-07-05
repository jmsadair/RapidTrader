#ifndef RAPID_TRADER_MARKET_H
#define RAPID_TRADER_MARKET_H
#include <iostream>
#include <robin_hood.h>
#include "order.h"
#include "sender.h"
#include "orderbook.h"

namespace RapidTrader::Matching {
class Market
{
public:
    explicit Market(Messaging::Sender outgoing_messenger_);
    Market(Market &&other) = delete;
    Market &operator=(Market &&other) = delete;
    void addOrderbook(uint32_t symbol_id);
    void deleteOrderbook(uint32_t symbol_id);
    void addSymbol(uint32_t symbol_id, std::string symbol_name);
    void deleteSymbol(uint32_t symbol_id);
    void addOrder(const Order &order);
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint32_t price);
    void executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity);
    void cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity);
    void deleteOrder(uint32_t symbol_id, uint64_t order_id);
    void replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint32_t new_price);

private:
    void addLimitOrder(OrderBook *book, Order order);
    void addMarketOrder(OrderBook *book, Order order);
    // Maps symbol to orderbook.
    std::vector<std::unique_ptr<OrderBook>> symbol_to_book;
    // Symbol IDs to symbol names.
    std::unordered_map<uint32_t, std::string> id_to_symbol;
    // Sends outgoing messages regarding the statuses of orders.
    Messaging::Sender outgoing_messenger;
};
} // namespace RapidTrader::Matching
#endif // RAPID_TRADER_MARKET_H
