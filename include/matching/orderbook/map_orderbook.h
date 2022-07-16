#ifndef RAPID_TRADER_MAP_ORDERBOOK_H
#define RAPID_TRADER_MAP_ORDERBOOK_H
#include <map>
#include <limits>
#include "robin_hood.h"
#include "level.h"
#include "orderbook.h"
#include "concurrent/messaging/sender.h"
#include "order.h"

// Only check the orderbook invariants in debug mode.
#ifndef NDEBUG
#    define BOOK_CHECK_INVARIANTS checkInvariants()
#else
#    define BOOK_CHECK_INVARIANTS
#endif

struct OrderWrapper
{
    Order order;
    // An iterator to the level that the order is stored in.
    // Used for finding the level to delete the order from in constant time.
    // Be careful about iterator invalidation! For the STL map, this iterator
    // will remain valid as long as element that iterator corresponds to in
    // the map is deleted. Insertions and deletions do not invalidate the iterator.
    std::map<uint64_t, Level>::iterator level_it;
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
    void executeOrder(uint64_t order_id, uint64_t quantity, uint64_t price) override;

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
     * Inserts a trailing stop order into the book.
     *
     * @param order the trailing stop order to insert.
     */
    void insertTrailingStopOrder(const Order &order);

    /**
     * Submits a stop market order to the book.
     *
     * @param order the stop market order to submit to the book.
     */
    void addStopOrder(Order &order);

    /**
     * Inserts a stop order into the book.
     *
     * @param order the stop order to insert.
     */
    void insertStopOrder(const Order &order);

    /**
     * Updates the stop price of trailing stop orders on the bid side.
     */
    void updateBidStopOrders();

    /**
     * Updates the stop price of trailing stop orders on the ask side.
     */
    void updateAskStopOrders();

    /**
     * Activates stop limit and stop market orders if the last traded
     * price is suitable.
     */
    void activateStopOrders();

    /**
     * Attempts to activate stop, stop limit, trailing stop, and trailing stop limit
     * orders on the bid side.
     *
     * @return true if any stop orders were activated and false otherwise.
     */
    bool activateBidStopOrders();

    /**
     * Attempts to activate stop, stop limit, trailing stop, and trailing stop limit
     * orders on the ask side.
     *
     * @return true if any stop orders were activated and false otherwise.
     */
    bool activateAskStopOrders();

    /**
     * Removes the provided order from the orderbook, converts it into a market
     * or limit order, and submits it for matching.
     *
     * @param order the order to activate, require that order is has a stop or
     *              stop limit type and is in the orderbook.
     */
    void activateStopOrder(Order order);

    /**
     * Matches all crossed orders in the book. Orders that are filled
     * are removed from the book.
     *
     * @param order the order to match.
     */
    void match(Order &order);

    /**
     * Indicates whether an order is able to completely filled
     * or not.
     *
     * @param order an order.
     * @return true if the order can be completely filled and false
     *         otherwise.
     */
    [[nodiscard]] bool canMatchOrder(const Order &order) const;

    /**
     * Matches two orders.
     *
     * @param ask an ask order to execute.
     * @param bid a bid order to execute.
     * @param executing_price price at which orders are executed, require that
     *                        ask price <= executing_price <= bid price .
     */
    void executeOrders(Order &ask, Order &bid, uint64_t executing_price);

    /**
     * @returns the last traded price if any trades have been made and zero otherwise.
     */
    [[nodiscard]] inline uint64_t lastTradedPriceAsk() const
    {
        return last_traded_price;
    }

    /**
     * @returns the last traded price if any trades have been made otherwise the maximum 64-bit integer value.
     */
    [[nodiscard]] inline uint64_t lastTradedPriceBid() const
    {
        return last_traded_price == 0 ? std::numeric_limits<uint64_t>::max() : last_traded_price;
    }

    /**
     * @returns the previous last traded price if more than one trade has been made and zero otherwise.
     */
    [[nodiscard]] inline uint64_t previousLastTradedPriceAsk() const
    {
        return previous_last_traded_price;
    }

    /**
     * @returns the previous last traded price if more than one trade has been made and zero otherwise.
     */
    [[nodiscard]] inline uint64_t previousLastTradedPriceBid() const
    {
        return previous_last_traded_price == 0 ? std::numeric_limits<uint64_t>::max() : previous_last_traded_price;
    }

    /**
     * Enforces the representation invariants of the orderbook.
     */
    void checkInvariants() const;

    // IMPORTANT: Note that the declaration of orders MUST be
    // declared before the declaration of the price level vectors.
    // Class members are destroyed in the reverse order of their declaration,
    // so the price level vectors will be destroyed before orders. This is
    // required for the intrusive list.

    // Maps order IDs to order wrappers.
    robin_hood::unordered_map<uint64_t, OrderWrapper> orders;
    // Maps prices to limit levels.
    std::map<uint64_t, Level> ask_levels;
    std::map<uint64_t, Level> bid_levels;
    // Maps prices to stop levels.
    std::map<uint64_t, Level> stop_ask_levels;
    std::map<uint64_t, Level> stop_bid_levels;
    // Maps prices to trailing stop levels.
    std::map<uint64_t, Level> trailing_stop_ask_levels;
    std::map<uint64_t, Level> trailing_stop_bid_levels;
    // The current price of the symbol - based off the price that the
    // symbol was last traded at. Initially zero.
    uint64_t last_traded_price;
    // The previous price of the symbol. Initially zero.
    uint64_t previous_last_traded_price;
    // Sends notifications regarding the execution, deletion, and update of orders.
    Concurrent::Messaging::Sender &outgoing_messages;
    // The symbol ID associated with the book.
    uint32_t symbol_id;
};
#endif // RAPID_TRADER_MAP_ORDERBOOK_H
