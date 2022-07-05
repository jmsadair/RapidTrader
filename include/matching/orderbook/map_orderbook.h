#ifndef RAPID_TRADER_MAP_ORDERBOOK_H
#define RAPID_TRADER_MAP_ORDERBOOK_H
#include <map>
#include <limits>
#include "robin_hood.h"
#include "level.h"
#include "orderbook.h"
#include "market.h"

class MapOrderBook : public OrderBook
{
public:
    friend class RapidTrader::Matching::Market;
    explicit MapOrderBook(uint32_t symbol_id_);
    void addOrder(const Order &order) override;
    void deleteOrder(uint64_t order_id) override;
    bool processMatching(std::queue<Order> &processed_orders) override;
    bool processMatching(Order &order, std::queue<Order> &processed_orders) override;
    bool executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price) override;
    bool executeOrder(uint64_t order_id, uint64_t quantity) override;
    bool cancelOrder(uint64_t order_id, uint64_t quantity) override;
    [[nodiscard]] bool canProcess(const Order &order) const override;
    [[nodiscard]] inline bool empty() const override
    {
        return orders.empty();
    }
    [[nodiscard]] inline uint32_t bestAsk() const override
    {
        return ask_levels.empty() ? std::numeric_limits<uint32_t>::max() : ask_levels.begin()->second.getPrice();
    }
    [[nodiscard]] inline uint32_t bestBid() const override
    {
        return bid_levels.empty() ? 0 : bid_levels.rbegin()->second.getPrice();
    }
    [[nodiscard]] inline bool hasOrder(uint64_t order_id) const override
    {
        return orders.find(order_id) != orders.end();
    }
    [[nodiscard]] inline const Order &getOrder(uint64_t order_id) const override
    {
        return orders.find(order_id)->second;
    }

private:
    Level *addLevel(const Order &order);
    void deleteLevel(const Order &order);
    static void matchOrders(Order &incoming, Order &existing);

    // IMPORTANT: Note that the declaration of orders MUST be
    // declared before the declaration of the price level vectors.
    // Class members are destroyed in the reverse order of their declaration,
    // so the price level vectors will be destroyed before orders. This is
    // required for the intrusive list.
    robin_hood::unordered_map<uint64_t, Order> orders;
    std::map<uint32_t, Level> ask_levels;
    std::map<uint32_t, Level> bid_levels;
    uint32_t symbol_id;
};
#endif // RAPID_TRADER_MAP_ORDERBOOK_H
