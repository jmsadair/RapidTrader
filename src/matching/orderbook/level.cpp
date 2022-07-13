#include <iostream>
#include "level.h"

Level::Level(uint64_t price_, LevelSide side_, uint32_t symbol_id_)
    : price(price_)
    , side(side_)
    , symbol_id(symbol_id_)
{
    volume = 0;
}

void Level::addOrder(Order &order)
{
    assert(order.isAsk() ? side == LevelSide::Ask : side == LevelSide::Bid && "Order is on different side than level!");
    assert(order.getPrice() == price && "Order does not have same price as the level!");
    assert(order.getSymbolID() == symbol_id && "Order does not have the same symbol ID as the level!");
    volume += order.getOpenQuantity();
    orders.push_front(order);
}

void Level::popFront()
{
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.front();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_front();
};

void Level::popBack()
{
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.back();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_back();
}

// LCOV_EXCL_START
std::ostream &operator<<(std::ostream &os, const Level &level)
{
    os << "Level Symbol ID: " << level.symbol_id;
    os << "\nLevel Side: " << (level.isAsk() ? "ASK" : "BID");
    os << "\nLevel Price: " << level.price;
    os << "\nLevel Volume: " << level.volume << "\n";
    return os;
}
// LCOV_EXCL_STOP
