#include <gtest/gtest.h>
#include "level.h"

TEST(Level, AddingOrdersToLevelShouldWork1)
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

    // Create a level to add the order to.
    Level level(price1, LevelSide::Bid, symbol1);
    // Level should initially have 0 volume.
    ASSERT_EQ(level.getVolume(), 0);
    // Level should be empty.
    ASSERT_TRUE(level.empty());
    // Add the order.
    level.addOrder(order1);
    // Level should now have same volume as order quantity.
    ASSERT_EQ(level.getVolume(), quantity1);
    // Level should have size of 1 since it contains only 1 order.
    ASSERT_EQ(level.size(), 1);
    // Level should now be non-empty.
    ASSERT_FALSE(level.empty());
}

TEST(Level, PoppingOrdersFromLevelShouldWork1)
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

    // Create a level to add the order to.
    Level level(price1, LevelSide::Bid, symbol1);
    // Level should initially have 0 volume.
    ASSERT_EQ(level.getVolume(), 0);
    // Level should be empty.
    ASSERT_TRUE(level.empty());
    // Add the order.
    level.addOrder(order1);
    // Level should now have same volume as order quantity.
    ASSERT_EQ(level.getVolume(), quantity1);
    // Level should have size of 1 since it contains only 1 order.
    ASSERT_EQ(level.size(), 1);
    // Level should now be non-empty.
    ASSERT_FALSE(level.empty());
    // Remove the order.
    level.popFront();
    // Level should now be empty again.
    ASSERT_TRUE(level.empty());
    // Level volume should be 0 again.
    ASSERT_EQ(level.getVolume(), 0);
}