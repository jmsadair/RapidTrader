#include "market_test_fixture.h"

/**
 * Tests replacing an order such that it does not result in matching.
 */
TEST_F(MarketTest, ReplaceOrderShouldWork1)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 1000;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t new_order_id = 2;
    uint64_t new_order_price = 1200;
    market.replaceOrder(symbol_id, id1, new_order_id, new_order_price);

    event_handler.stop();

    checkOrderAdded(id1);
    checkOrderAdded(new_order_id);
    checkOrderDeleted(id1, 0, 0, quantity1);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests replacing an order such that it does result in matching.
 */
TEST_F(MarketTest, ReplaceOrderShouldWork2)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 100;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    OrderTimeInForce tof2 = OrderTimeInForce::GTC;
    uint64_t quantity2 = 1000;
    uint64_t price2 = 1200;
    uint64_t id2 = 2;
    Order order2 = Order::limitBidOrder(id2, symbol_id, price2, quantity2, tof2);
    market.addOrder(order2);

    OrderTimeInForce tof3 = OrderTimeInForce::GTC;
    uint64_t quantity3 = 500;
    uint64_t price3 = 2000;
    uint64_t id3 = 3;
    Order order3 = Order::limitAskOrder(id3, symbol_id, price3, quantity3, tof3);
    market.addOrder(order3);

    uint64_t new_order_id = 4;
    uint64_t new_order_price = 900;
    market.replaceOrder(symbol_id, id3, new_order_id, new_order_price);

    event_handler.stop();

    checkOrderAdded(id1);
    checkOrderAdded(id2);
    checkOrderAdded(id3);
    checkOrderAdded(new_order_id);
    checkExecutedOrder(id1, price1, quantity1, 0);
    checkExecutedOrder(new_order_id, price1, quantity1, quantity3 - quantity1);
    checkExecutedOrder(id2, price2, quantity3 - quantity1, quantity2 - (quantity3 - quantity1));
    checkExecutedOrder(new_order_id, price2, quantity3 - quantity1, 0);
    checkOrderDeleted(id3, 0, 0, quantity3);
    checkOrderDeleted(id1, price1, quantity1, 0);
    checkOrderDeleted(new_order_id, price2, quantity3 - quantity1, 0);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests trying to replace an invalid order returns an error (i.e. the orderbook does not exists,
 * the symbol does not exists, the order does not exist, etc.).
 */
TEST_F(MarketTest, ReplaceOrderShouldWork3)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 100;
    uint64_t price1 = 1500;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint32_t invalid_symbol_id = 0;
    uint64_t invalid_price = 0;
    uint64_t invalid_id = 0;

    // Replace the order with invalid symbol.
    ASSERT_EQ(market.replaceOrder(invalid_symbol_id, id1, id1, price1), ErrorStatus::SymbolDoesNotExist);
    // Replace the order with invalid replacement order ID.
    ASSERT_EQ(market.replaceOrder(symbol_id, id1, invalid_id, price1), ErrorStatus::InvalidOrderID);
    // Replace the order with invalid price.
    ASSERT_EQ(market.replaceOrder(symbol_id, id1, id1, invalid_price), ErrorStatus::InvalidPrice);
    // Replace the order with ID that does not exist.
    ASSERT_EQ(market.replaceOrder(symbol_id, invalid_id, id1, price1), ErrorStatus::OrderDoesNotExist);

    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";
    market.addSymbol(new_symbol_id, new_symbol_name);

    ASSERT_EQ(market.replaceOrder(new_symbol_id, id1, id1, price1), ErrorStatus::OrderBookDoesNotExist);

    event_handler.stop();

    checkOrderAdded(id1);
    checkSymbolAdded(new_symbol_id, new_symbol_name);
    ASSERT_TRUE(event_handler.empty());
}