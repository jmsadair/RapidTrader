#ifndef FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#define FAST_EXCHANGE_VECTOR_ORDERBOOK_H
#include <vector>
#include <limits>
#include "robin_hood.h"
#include "price_level.h"
#include "event.h"
#include "receiver.h"
#include "orderbook_interface.h"

constexpr size_t DEFAULT_PRICE_LEVELS_SIZE = 1000;

namespace OrderBook {
    /**
     * An implementation of OrderBook using vectors to represent the book. Each index of
     * ask_price_levels and bid_price_levels represents a price_level, max_bid_price represents
     * the current maximum bid price, and min_ask_price represents the current minimum asking price.
     *
     * Invariants:
     * 1. For any i such that 0 < i < min_ask_price, ask_price_levels[i].orders
     *    must be empty. That is, there must not be any ask orders that have a lower
     *    price than the current minimum asking price. Furthermore, ask_price_levels[i].orders
     *    must only contain ask orders.
     * 2. For any i such that max_bid_price < i < ask_price_levels.size(), bid_price_levels[i].orders
     *    must be empty. That is, there must not be any bid orders that have a higher
     *    price than the current maximum asking price. Furthermore, bid_price_levels[i].orders
     *    must only contain bid orders.
     * 3. Any value that is in orders must also be in either in ask_price_levels or bid_price_levels.
     * 4. All orders in a price level must have the same price.
     * 5. All orders in a price level must be on the same side - all orders are ask or all are bid.
     * 6. The sum of the quantities of the orders in a price level must be equal to the price level volume.
     * 7. The orders in a price level must be GTC orders, there cannot be any FOK or IOC orders in the price level.
     */
    class VectorOrderBook : public OrderBook {
    public:
        /**
         * A constructor for the VectorOrderBook ADT.
         *
         * @param symbol_ the symbol associated with the order book.
         * @param outgoing_ a message sender used to communicate with users.
         */
        inline VectorOrderBook(uint32_t symbol_id_, Messaging::Sender outgoing_) :
            symbol_id(symbol_id_), outgoing(outgoing_)
        {
            ask_price_levels.resize(DEFAULT_PRICE_LEVELS_SIZE);
            bid_price_levels.resize(DEFAULT_PRICE_LEVELS_SIZE);
        }

       /**
        * @inheritdoc
        */
        void placeOrder(Order &order) override;

        /**
         * @inheritdoc
         */
        void cancelOrder(uint64_t order_id) override;

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline bool hasOrder(uint64_t order_id) const override { return orders.find(order_id) != orders.end(); }

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline const Order& getOrder(uint64_t order_id) const override { return orders.at(order_id); }

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline uint32_t minAskPrice() const override { return min_ask_price; }

        /**
         * @inheritdoc
         */
        [[nodiscard]] inline uint32_t maxBidPrice() const override { return max_bid_price; }

        /**
        * Verifies that the order book satisfies its representation invariants.
        *
        * @throws Error if any of the price levels or in an invalid state or if there exists an
         *              order in orders that is not also in a price level.
        */
        void verifyOrderBookState() const;

    private:
        /**
         * @inheritdoc
         */
        void execute(Order &incoming, Order &existing) override;

        /**
         * @inheritdoc
         */
        void executePriceChain(const std::vector<uint32_t> &price_chain, Order &order) override;

        /**
         * @inheritdoc
         */
        std::pair<std::vector<uint32_t>, bool> getPriceChain(const Order &order) override;

        /**
         * @inheritdoc
         */
        void match(Order &order)  override;

        /**
         * @inheritdoc
         */
        void placeGtcOrder(Order &order) override;

        /**
         * @inheritdoc
         */
        void placeFokOrder(Order &order) override;

        /**
         * @inheritdoc;
         */
        void placeIocOrder(Order &order) override;

        /**
         * @inheritdoc
         */
        void insert(const Order &order) override;

        /**
         * @inheritdoc
         */
        void remove(const Order &order) override;

        /**
         * Verifies the validity of a price level in the order book.
         *
         * @param price_level the price level to verify.
         * @param expected_price the expected price of all orders in the price level.
         * @param expected_order_side the expected side - ask or bid - of all orders in the price level.
         * @throws Error if not all orders in the price level have the same price, not all order are GTC orders,
         *               not all orders are on the same side, or if the price level corresponds to a price lower
         *               than the minimum asking price (or greater than the maximum bidding price) and is non-empty.
         */
        void verifyPriceLevelState(const PriceLevel &price_level, uint32_t expected_price,
                                   OrderSide expected_order_side) const;

        // IMPORTANT: Note that the declaration of orders MUST be
        // declared before the declaration of the price level vectors.
        // Class members are destroyed in the reverse order of their declaration,
        // so the price level vectors will be destroyed before orders. This is
        // required for the intrusive list.
        robin_hood::unordered_map<uint64_t, Order> orders;
        std::vector<PriceLevel> ask_price_levels;
        std::vector<PriceLevel> bid_price_levels;
        uint32_t min_ask_price = std::numeric_limits<uint32_t>::max();
        uint32_t max_bid_price = 0;
        uint32_t symbol_id;
        Messaging::Sender outgoing;
    };
}

#endif //FAST_EXCHANGE_VECTOR_ORDERBOOK_H
