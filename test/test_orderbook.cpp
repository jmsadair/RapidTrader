#include <gtest/gtest.h>
#include <queue>
#include "map_orderbook.h"
#include "order.h"

TEST(OrderBookTest, BookEmptyShoudWork1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};
    // Book should be empty.
    ASSERT_TRUE(book.empty());
}

TEST(OrderBookTest, BookIsEmptyShoudWork1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Book should not be empty.
    ASSERT_FALSE(book.empty());
}

TEST(OrderBookTest, BookBestAskPriceShouldWork1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Best ask price should be 0 when book is empty.
    ASSERT_EQ(book.bestAsk(), std::numeric_limits<uint32_t>::max());

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Best ask price should now be thew price of the order that was added.
    ASSERT_EQ(book.bestAsk(), price1);
}

TEST(OrderBookTest, BookExecuteWorks1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;
    uint32_t executed_price = 250;

    // Execute the order.
    book.executeOrder(id1, executed_quantity, executed_price);

    // Open quantity should now be 100.
    ASSERT_EQ(book.getOrder(id1).getOpenQuantity(), quantity1 - executed_quantity);
    // Executed quantity should now be 100.
    ASSERT_EQ(book.getOrder(id1).getExecutedQuantity(), executed_quantity);
    // Last executed price should now be 250.
    ASSERT_EQ(book.getOrder(id1).getLastExecutedPrice(), executed_price);
}

TEST(OrderBookTest, BookExecuteWorks2)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Execution data.
    uint64_t executed_quantity = 100;

    // Execute the order.
    book.executeOrder(id1, executed_quantity);

    // Open quantity should now be 100.
    ASSERT_EQ(book.getOrder(id1).getOpenQuantity(), quantity1 - executed_quantity);
    // Executed quantity should now be 100.
    ASSERT_EQ(book.getOrder(id1).getExecutedQuantity(), executed_quantity);
    // Last executed price should be the price of the order.
    ASSERT_EQ(book.getOrder(id1).getLastExecutedPrice(), price1);
}

TEST(OrderBookTest, BookCancelWorks1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Quantity to cancel.
    uint64_t cancel_quantity = 100;

    // Cancel the order.
    book.cancelOrder(id1, cancel_quantity);

    // Open quantity should now be 100.
    ASSERT_EQ(book.getOrder(id1).getOpenQuantity(), quantity1 - cancel_quantity);
    // Executed quantity should still be 0.
    ASSERT_EQ(book.getOrder(id1).getExecutedQuantity(), 0);
}

TEST(OrderBookTest, BookDeleteWorks1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Create LIMIT BID GTC order.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 400;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol, price2, quantity2, id2};
    book.addOrder(order2);

    // Delete the second order.
    book.deleteOrder(id2);

    // Order should not be in book.
    ASSERT_FALSE(book.hasOrder(id2));
    // Best price should be price of first order.
    ASSERT_EQ(book.bestAsk(), price1);
}

TEST(OrderBookTest, BookBestBidPriceShouldWork1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Best bid price should be 0 when book is empty.
    ASSERT_EQ(book.bestBid(), 0);

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // Best ask price should now be thew price of the order that was added.
    ASSERT_EQ(book.bestBid(), price1);
}

TEST(OrderBookTest, BookShouldMatch1)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create a queue to monitor processed orders.
    std::queue<Order> processed_orders;

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 200;
    uint32_t price1 = 350;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT ASK GTC order.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 100;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol, price2, quantity2, id2};
    book.addOrder(order2);

    // Orders should have been matched.
    ASSERT_TRUE(book.processMatching(processed_orders));

    // Order 1 match.
    Order &processed_order1 = processed_orders.front();
    ASSERT_EQ(processed_order1.getOrderID(), id1);
    ASSERT_EQ(processed_order1.getExecutedQuantity(), quantity2);
    ASSERT_EQ(processed_order1.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // Order 2 match.
    Order &processed_order2 = processed_orders.front();
    ASSERT_EQ(processed_order2.getOrderID(), id2);
    ASSERT_EQ(processed_order2.getExecutedQuantity(), quantity2);
    ASSERT_EQ(processed_order2.getLastExecutedPrice(), price1);

    processed_orders.pop();

    // No other orders should have been processed.
    ASSERT_TRUE(processed_orders.empty());
}

TEST(OrderBookTest, BookShouldMatch2)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create a queue to monitor processed orders.
    std::queue<Order> processed_orders;

    // Create LIMIT ASK GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 50;
    uint32_t price1 = 100;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT ASK GTC order.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 101;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol, price2, quantity2, id2};
    book.addOrder(order2);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT ASK GTC order.
    OrderAction action3 = OrderAction::Limit;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::GoodTillCancel;
    uint32_t quantity3 = 200;
    uint32_t price3 = 102;
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol, price3, quantity3, id3};
    book.addOrder(order3);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT BID GTC order.
    OrderAction action4 = OrderAction::Limit;
    OrderSide side4 = OrderSide::Bid;
    OrderType type4 = OrderType::GoodTillCancel;
    uint32_t quantity4 = 400;
    uint32_t price4 = 101;
    uint64_t id4 = 4;
    Order order4{action4, side4, type4, symbol, price4, quantity4, id4};
    book.addOrder(order4);

    // There should be matched orders.
    ASSERT_TRUE(book.processMatching(processed_orders));

    // Order 4 match.
    Order &processed_order1 = processed_orders.front();
    ASSERT_EQ(processed_order1.getOrderID(), id4);
    ASSERT_EQ(processed_order1.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order1.getLastExecutedPrice(), price1);
    processed_orders.pop();

    // Order 1 match.
    Order &processed_order2 = processed_orders.front();
    ASSERT_EQ(processed_order2.getOrderID(), id1);
    ASSERT_EQ(processed_order2.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order2.getLastExecutedPrice(), price4);
    processed_orders.pop();

    // Order 4 match.
    Order &processed_order3 = processed_orders.front();
    ASSERT_EQ(processed_order3.getOrderID(), id4);
    ASSERT_EQ(processed_order3.getExecutedQuantity(), quantity2 + quantity1);
    ASSERT_EQ(processed_order3.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // Order 2 match.
    Order &processed_order4 = processed_orders.front();
    ASSERT_EQ(processed_order4.getOrderID(), id2);
    ASSERT_EQ(processed_order4.getExecutedQuantity(), quantity2);
    ASSERT_EQ(processed_order4.getLastExecutedPrice(), price4);
    processed_orders.pop();

    // No other orders should have been processed.
    ASSERT_TRUE(processed_orders.empty());
}

TEST(OrderBookTest, BookShouldMatch3)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create a queue to monitor processed orders.
    std::queue<Order> processed_orders;

    // Create LIMIT ASK GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Ask;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 50;
    uint32_t price1 = 1;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT ASK GTC order.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Ask;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 100;
    uint32_t price2 = 2;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol, price2, quantity2, id2};
    book.addOrder(order2);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create MARKET BID IOC order.
    OrderAction action3 = OrderAction::Market;
    OrderSide side3 = OrderSide::Bid;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 150;
    uint32_t price3 = std::numeric_limits<uint32_t>::max();
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol, price3, quantity3, id3};

    // There should be matches.
    ASSERT_TRUE(book.processMatching(order3, processed_orders));

    // Order 3 match.
    Order &processed_order1 = processed_orders.front();
    ASSERT_EQ(processed_order1.getOrderID(), id3);
    ASSERT_EQ(processed_order1.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order1.getLastExecutedPrice(), price1);
    processed_orders.pop();

    // Order 1 match.
    Order &processed_order2 = processed_orders.front();
    ASSERT_EQ(processed_order2.getOrderID(), id1);
    ASSERT_EQ(processed_order2.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order2.getLastExecutedPrice(), price1);
    processed_orders.pop();

    // Order 3 match.
    Order &processed_order3 = processed_orders.front();
    ASSERT_EQ(processed_order3.getOrderID(), id3);
    ASSERT_EQ(processed_order3.getExecutedQuantity(), quantity1 + quantity2);
    ASSERT_EQ(processed_order3.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // Order 2 match.
    Order &processed_order4 = processed_orders.front();
    ASSERT_EQ(processed_order4.getOrderID(), id2);
    ASSERT_EQ(processed_order4.getExecutedQuantity(), quantity2);
    ASSERT_EQ(processed_order4.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // No other orders should have been processed.
    ASSERT_TRUE(processed_orders.empty());
}

TEST(OrderBookTest, BookShouldMatch4)
{
    uint32_t symbol = 1;
    MapOrderBook book{symbol};

    // Create a queue to monitor processed orders.
    std::queue<Order> processed_orders;

    // Create LIMIT BID GTC order.
    OrderAction action1 = OrderAction::Limit;
    OrderSide side1 = OrderSide::Bid;
    OrderType type1 = OrderType::GoodTillCancel;
    uint32_t quantity1 = 50;
    uint32_t price1 = 50;
    uint64_t id1 = 1;
    Order order1{action1, side1, type1, symbol, price1, quantity1, id1};
    book.addOrder(order1);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create LIMIT BID GTC order.
    OrderAction action2 = OrderAction::Limit;
    OrderSide side2 = OrderSide::Bid;
    OrderType type2 = OrderType::GoodTillCancel;
    uint32_t quantity2 = 1;
    uint32_t price2 = 25;
    uint64_t id2 = 2;
    Order order2{action2, side2, type2, symbol, price2, quantity2, id2};
    book.addOrder(order2);

    // There should be no orders to match with.
    ASSERT_FALSE(book.processMatching(processed_orders));

    // Create MARKET ASK IOC order.
    OrderAction action3 = OrderAction::Market;
    OrderSide side3 = OrderSide::Ask;
    OrderType type3 = OrderType::ImmediateOrCancel;
    uint32_t quantity3 = 200;
    uint32_t price3 = std::numeric_limits<uint32_t>::min();
    uint64_t id3 = 3;
    Order order3{action3, side3, type3, symbol, price3, quantity3, id3};

    // There should be matches.
    ASSERT_TRUE(book.processMatching(order3, processed_orders));

    // Order 1 match.
    Order &processed_order1 = processed_orders.front();
    ASSERT_EQ(processed_order1.getOrderID(), id1);
    ASSERT_EQ(processed_order1.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order1.getLastExecutedPrice(), price1);
    processed_orders.pop();

    // Order 3 match.
    Order &processed_order2 = processed_orders.front();
    ASSERT_EQ(processed_order2.getOrderID(), id3);
    ASSERT_EQ(processed_order2.getExecutedQuantity(), quantity1);
    ASSERT_EQ(processed_order2.getLastExecutedPrice(), price1);
    processed_orders.pop();

    // Order 2 match.
    Order &processed_order4 = processed_orders.front();
    ASSERT_EQ(processed_order4.getOrderID(), id2);
    ASSERT_EQ(processed_order4.getExecutedQuantity(), quantity2);
    ASSERT_EQ(processed_order4.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // Order 3 match.
    Order &processed_order3 = processed_orders.front();
    ASSERT_EQ(processed_order3.getOrderID(), id3);
    ASSERT_EQ(processed_order3.getExecutedQuantity(), quantity1 + quantity2);
    ASSERT_EQ(processed_order3.getLastExecutedPrice(), price2);
    processed_orders.pop();

    // No other orders should have been processed.
    ASSERT_TRUE(processed_orders.empty());
}
