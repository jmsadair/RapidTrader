#include <gtest/gtest.h>
#include "order.h"

TEST(Order, OrderConstructorShouldWork)
{
    // Create a LIMIT BID GTC order.
    uint32_t symbol1 = 1;
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 100;
    uint32_t price1 = 100;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol1, price1, quantity1, id1};

    // Symbol ID should be same as provided symbol ID.
    ASSERT_EQ(order1.getSymbolID(), symbol1);
    // Order ID should be same as provided order ID.
    ASSERT_EQ(order1.getOrderID(), id1);
    // Quantity should be same as provided quantity.
    ASSERT_EQ(order1.getQuantity(), quantity1);
    // Price should be same as provided price.
    ASSERT_EQ(order1.getQuantity(), price1);
    // Executed quantity should be 0 for new order.
    ASSERT_EQ(order1.getExecutedQuantity(), 0);
    // Last executed quantity should be 0 for new order.
    ASSERT_EQ(order1.getLastExecutedPrice(), 0);
    // Open quantity should be same as quantity for new order.
    ASSERT_EQ(order1.getOpenQuantity(), quantity1);
    // Order should be a limit order.
    ASSERT_TRUE(order1.isLimit());
    // Order should be on the bid side.
    ASSERT_TRUE(order1.isBid());
    // Order should be a GTC order.
    ASSERT_TRUE(order1.isGtc());
}

TEST(Order, OrderExecutionShouldWork)
{
    // Create a MARKET ASK IOC order.
    uint32_t symbol1 = 1;
    OrderAction action1 = OrderAction::Market;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::ImmediateOrCancel;
    uint32_t quantity1 = 100;
    uint32_t price1 = 100;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol1, price1, quantity1, id1};

    // Execution price and quantity.
    uint32_t executed_price = 150;
    uint64_t executed_quantity = 25;

    // Execute the order.
    order1.execute(executed_price, executed_quantity);

    ASSERT_EQ(order1.getExecutedQuantity(), executed_quantity);
    ASSERT_EQ(order1.getLastExecutedPrice(), executed_price);
    ASSERT_EQ(order1.getOpenQuantity(), quantity1 - executed_quantity);
}
