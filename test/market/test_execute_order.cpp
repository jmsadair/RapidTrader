#include "market_test_fixture.h"
/**
 * Tests Executing an order with a provided quantity but not a price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork1)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t executed_quantity = 100;
    market.executeOrder(symbol_id, id1, executed_quantity);

    event_handler.stop();

    checkOrderAdded(id1);
    checkExecutedOrder(id1, price1, executed_quantity, quantity1 - executed_quantity);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity and price.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork2)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t executed_quantity = 100;
    uint32_t executed_price = 400;
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    event_handler.stop();

    checkOrderAdded(id1);
    checkExecutedOrder(id1, executed_price, executed_quantity, quantity1 - executed_quantity);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork3)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1); // Add the order.
    market.addOrder(order1);

    uint64_t executed_quantity = 200;
    market.executeOrder(symbol_id, id1, executed_quantity);

    event_handler.stop();

    checkOrderAdded(id1);
    checkExecutedOrder(id1, price1, executed_quantity, 0);
    checkOrderDeleted(id1, price1, executed_quantity, 0);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests Executing an order with a provided quantity and price such that it must be deleted.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork4)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 350;
    uint64_t id1 = 1;
    Order order1 = Order::limitBidOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t executed_quantity = 200;
    uint64_t executed_price = 300;
    market.executeOrder(symbol_id, id1, executed_quantity, executed_price);

    event_handler.stop();

    checkOrderAdded(id1);
    checkExecutedOrder(id1, executed_price, executed_quantity, 0);
    checkOrderDeleted(id1, executed_price, executed_quantity, 0);
    ASSERT_TRUE(event_handler.empty());
}

/**
 * Tests trying to execute order with invalid parameters returns error.
 */
TEST_F(MarketTest, ExecuteOrderShouldWork5)
{
    OrderTimeInForce tof1 = OrderTimeInForce::GTC;
    uint64_t quantity1 = 200;
    uint64_t price1 = 100;
    uint64_t id1 = 1;
    Order order1 = Order::limitAskOrder(id1, symbol_id, price1, quantity1, tof1);
    market.addOrder(order1);

    uint64_t invalid_executed_quantity = 0;
    uint64_t invalid_executed_price = 0;
    uint64_t invalid_executed_id = 0;
    uint64_t invalid_symbol_id = 2;

    // Execute order with invalid quantity.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, invalid_executed_quantity), ErrorStatus::InvalidQuantity);
    // Execute order with invalid price.
    ASSERT_EQ(market.executeOrder(symbol_id, id1, quantity1, invalid_executed_price), ErrorStatus::InvalidPrice);
    // Execute order with invalid ID - does not exist.
    ASSERT_EQ(market.executeOrder(symbol_id, invalid_executed_id, quantity1, price1), ErrorStatus::OrderDoesNotExist);
    // Executed order with invalid symbol ID - does not exist.
    ASSERT_EQ(market.executeOrder(invalid_symbol_id, id1, quantity1, price1), ErrorStatus::SymbolDoesNotExist);

    uint32_t new_symbol_id = 2;
    std::string new_symbol_name = "MSFT";
    market.addSymbol(new_symbol_id, new_symbol_name);
    ASSERT_TRUE(market.hasSymbol(new_symbol_id));

    // Execute with invalid orderbook ID - does not exist.
    // Executed order with invalid symbol ID - does not exist.
    ASSERT_EQ(market.executeOrder(invalid_symbol_id, id1, quantity1, price1), ErrorStatus::OrderBookDoesNotExist);

    event_handler.stop();

    checkOrderAdded(id1);
    checkSymbolAdded(new_symbol_id, new_symbol_name);
    ASSERT_TRUE(event_handler.empty());
}