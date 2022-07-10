#ifndef RAPID_TRADER_MAP_ORDERBOOK_H
#define RAPID_TRADER_MAP_ORDERBOOK_H
#include <map>
#include <limits>
#include "robin_hood.h"
#include "level.h"
#include "orderbook.h"
#include "concurrent/messaging/sender.h"

struct OrderWrapper
{
    Order order;
    // An iterator to the level that the order is stored in.
    // Used for finding the level to delete the order from in constant time.
    // Be careful about iterator invalidation! For the STL map, this iterator
    // will remain valid as long as element that iterator corresponds to in
    // the map is deleted. Insertions and deletions do not invalidate the iterator.
    std::map<uint32_t, Level>::iterator level_it;
};

class MapOrderBook : public OrderBook
{
public:
    /**
     * A constructor for the MapOrderBook.
     *
     * @param symbol_id_ the symbol ID that will be associated with the book.
     * @param outgoing_messages_ a message queue for outgoing messages.
     */
    MapOrderBook(uint32_t symbol_id_, Concurrent::Messaging::Sender &outgoing_messages_);

    /**
     * @inheritdoc
     */
    void addOrder(Order order) override;

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
    void replaceOrder(uint64_t order_id, uint64_t new_order_id, uint64_t new_price) override;

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline uint32_t marketAskPrice() const override
    {
        uint32_t best_ask = ask_levels.empty() ? std::numeric_limits<uint32_t>::max() : ask_levels.begin()->first;
        return std::min(best_ask, ask_match_price);
    }

    /**
     * @inheritdoc
     */
    [[nodiscard]] inline uint32_t marketBidPrice() const override
    {
        uint32_t best_bid = bid_levels.empty() ? 0 : bid_levels.rbegin()->first;
        return std::max(best_bid, bid_match_price);
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
        return orders.find(order_id)->second.order;
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
     * Deletes an order from the book. Does not match orders.
     *
     * @param order_id the ID of the order, require that there exists
     *                 an order with order_id in the book.
     * @param notification true if a notification should be sent
     *                     indicating that the order was deleted and
     *                     false otherwise.
     */
    void deleteOrder(uint64_t order_id, bool notification);

    /**
     * Submits a limit order to the book.
     *
     * @param order the limit order to add to the book, require that the order
     *              does not already exist in the book.
     */
    void addLimitOrder(Order &order);

    /**
     * Inserts a limit order into the book.
     *
     * @param order the order to insert.
     */
    void insertLimitOrder(const Order &order);

    /**
     * Submits a market order to the book.
     *
     * @param order the market order to add to the book.
     */
    void addMarketOrder(Order &order);

    /**
     * Submits a stop market order to the book.
     *
     * @param order the stop market order to submit to the book.
     */
    void addStopOrder(Order &order);

    /**
     * Inserts a stop order into the book.
     *
     * @param order the order to insert, require that order has stop
     *              or stop limit action.
     */
    void insertStopOrder(const Order &order);

    /**
     * Activates stop limit and stop market orders if possible.
     *
     * @return true if orders were activated and false otherwise.
     */
    bool activateStopOrders();

    /**
     * Removes the provided order from the orderbook, converts it into a market
     * or limit order, and submits it for matching.
     *
     * @param order the order to activate, require that order is has a stop or
     *              stop limit action and is in the orderbook.
     */
    void activateStopOrder(Order order);

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
    void matchOrders(Order &ask, Order &bid);

    /**
     * Updates the last ask and bid match prices.
     *
     * @param order the order to update the match price with.
     */
    inline void updateMatchingPrice(const Order &order) {
        if (order.isAsk())
            ask_match_price = order.getPrice();
        else
            bid_match_price = order.getPrice();
    }

    /**
     * Resets the last ask match price to its maximum value and
     * the last bid match price to its minimum value.
     */
    inline void resetMatchingPrice() {
        ask_match_price = std::numeric_limits<uint32_t>::max();
        bid_match_price = 0;
    }

    // IMPORTANT: Note that the declaration of orders MUST be
    // declared before the declaration of the price level vectors.
    // Class members are destroyed in the reverse order of their declaration,
    // so the price level vectors will be destroyed before orders. This is
    // required for the intrusive list.

    // Maps order IDs to order wrappers.
    robin_hood::unordered_map<uint64_t, OrderWrapper> orders;
    // Maps prices to levels.
    std::map<uint32_t, Level> ask_levels;
    std::map<uint32_t, Level> bid_levels;
    std::map<uint32_t, Level> stop_ask_levels;
    std::map<uint32_t, Level> stop_bid_levels;
    // Last matched prices.
    uint32_t ask_match_price;
    uint32_t bid_match_price;
    Concurrent::Messaging::Sender &outgoing_messages;
    // The symbol ID associated with the book.
    uint32_t symbol_id;
};
#endif // RAPID_TRADER_MAP_ORDERBOOK_H
