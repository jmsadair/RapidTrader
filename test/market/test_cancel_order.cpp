#include "market_test_fixture.h"

/**
 * Tests cancelling a portion of an order's quantity such that it is not removed from the book.
 */
TEST_F(MarketTest, CancelOrderShouldWork1)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t cancel_quantity = 100;

    market.cancelOrder(symbol_id, id1, cancel_quantity);

    event_handler.stop();

    checkOrderAdded(id1);
    checkOrderUpdated(id1, 0, 0, quantity1 - cancel_quantity);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests cancelling a portion of an order's quantity such that it is removed from the book.
 */
TEST_F(MarketTest, CancelOrderShouldWork2)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t cancel_quantity = 200;

    market.cancelOrder(symbol_id, id1, cancel_quantity);

    event_handler.stop();

    checkOrderAdded(id1);
    checkOrderUpdated(id1, 0, 0, 0);
    checkOrderDeleted(id1, 0, 0, 0);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests cancelling an order with invalid parameters returns an error.
 */
TEST_F(MarketTest, CancelOrderShouldWork3)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    // Invalid cancellation data.
    uint64_t invalid_cancel_quantity = 0;
    uint64_t invalid_cancel_id = 0;
    uint64_t invalid_symbol_id = 0;

    // Cancel the order with invalid quantity.
    ASSERT_EQ(market.cancelOrder(symbol_id, id1, invalid_cancel_quantity), ErrorStatus::InvalidQuantity);
    // Cancel the order with invalid order ID - order does not exist.
    ASSERT_EQ(market.cancelOrder(symbol_id, invalid_cancel_id, quantity1), ErrorStatus::OrderDoesNotExist);
    // Cancel the order with invalid symbol ID - symbol does not exist.
    ASSERT_EQ(market.cancelOrder(invalid_symbol_id, quantity1, quantity1), ErrorStatus::SymbolDoesNotExist);

    // Symbol data.
    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";

    // Add the symbol but not the orderbook.
    market.addSymbol(new_symbol_id, new_symbol_name);

    // Cancel with invalid orderbook ID - orderbook does not exist.
    ASSERT_EQ(market.cancelOrder(new_symbol_id, id1, quantity1), ErrorStatus::OrderBookDoesNotExist);

    event_handler.stop();

    checkOrderAdded(id1);
    checkSymbolAdded(new_symbol_id, new_symbol_name);
    ASSERT_TRUE(event_handler.empty());
}
