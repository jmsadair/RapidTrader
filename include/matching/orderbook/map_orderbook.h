#ifndef RAPID_TRADER_MAP_ORDERBOOK_H
#define RAPID_TRADER_MAP_ORDERBOOK_H
#include <map>
#include <limits>
#include "robin_hood.h"
#include "level.h"
#include "orderbook.h"
#include "sender.h"

class MapOrderBook : public OrderBook
{
public:
    /**
     * A constructor for the MapOrderBook.
     *
     * @param symbol_id_ the symbol ID that will be associated with the book.
     * @param outgoing_messages_ a message queue for outgoing messages.
     */
    MapOrderBook(uint32_t symbol_id_, Messaging::Sender &outgoing_messages_);

    /**
     * @inheritdoc
     */
    void addLimitOrder(Order order) override;

    /**
     * @inheritdoc
     */
    void addMarketOrder(Order order) override;

    /**
     * @inheritdoc
     */
    void executeOrder(uint64_t order_id, uint64_t quantity, uint32_t price) override;

    /**
     * @inheritdoc
     */
    void executeOrder(uint64_t order_id, uint64_t quantity) override;

    /**
     * @inheritdoc
     */
    void deleteOrder(uint64_t order_id) override;

    /**
     * @inheritdoc
     */
    void cancelOrder(uint64_t order_id, uint64_t quantity) override;

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline uint32_t marketAskPrice() const override
    {
        return ask_levels.empty() ? std::numeric_limits<uint32_t>::max() : ask_levels.begin()->second.getPrice();
    }

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline uint32_t marketBidPrice() const override
    {
        return bid_levels.empty() ? 0 : bid_levels.rbegin()->second.getPrice();
    }

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline bool hasOrder(uint64_t order_id) const override
    {
        return orders.find(order_id) != orders.end();
    }

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline const Order &getOrder(uint64_t order_id) const override
    {
        return orders.find(order_id)->second;
    }

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline bool empty() const override
    {
        return orders.empty();
    }
private:
    /**
     * Adds a new price level to the book.
     *
     * @param order the initial order that will be added to the price level.
     * @return a pointer the newly added level.
     */
    Level *addLevel(const Order &order);

    /**
     * Deletes a price level from the book.
     *
     * @param order an order that is the level to be deleted.
     */
    void deleteLevel(const Order &order);

    /**
     * Indicates whether an order can be filled or not.
     *
     * @param order the order.
     * @return true if the order can be filled and false otherwise.
     */
    [[nodiscard]] bool canProcess(const Order &order) const;

    /**
     * Matches all crossed orders in the book. Orders that are filled
     * are removed from the book.
     */
    void processMatching();

    /**
     * Matches all crossed orders in the book. Orders that are filled
     * are removed from the book.
     *
     * @param order the order to match.
     */
    void processMatching(Order &order);

    /**
     * Matches two orders.
     *
     * @param ask an ask order to match.
     * @param bid a bid order to match.
     */
    static void matchOrders(Order &ask, Order &bid);

    // IMPORTANT: Note that the declaration of orders MUST be
    // declared before the declaration of the price level vectors.
    // Class members are destroyed in the reverse order of their declaration,
    // so the price level vectors will be destroyed before orders. This is
    // required for the intrusive list.
    robin_hood::unordered_map<uint64_t, Order> orders;
    std::map<uint32_t, Level> ask_levels;
    std::map<uint32_t, Level> bid_levels;
    Messaging::Sender &outgoing_messages;
    uint32_t symbol_id;
};
#endif // RAPID_TRADER_MAP_ORDERBOOK_H
