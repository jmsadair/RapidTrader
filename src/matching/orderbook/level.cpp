#include <iostream>
#include "level.h"

Level::Level(uint64_t price_, LevelSide side_, uint32_t symbol_id_)
    : price(price_)
    , side(side_)
    , symbol_id(symbol_id_)
{
    volume = 0;
}

const list<Order> &Level::getOrders() const
{
    return orders;
}

list<Order> &Level::getOrders()
{
    return orders;
}

void Level::addOrder(Order &order)
{
    assert(order.isAsk() ? side == LevelSide::Ask : side == LevelSide::Bid && "Order is on different side than level!");
    assert(order.getSymbolID() == symbol_id && "Order does not have the same symbol ID as the level!");
    volume += order.getOpenQuantity();
    orders.push_front(order);
    LEVEL_CHECK_INVARIANTS;
}

void Level::popFront()
{
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.front();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_front();
    LEVEL_CHECK_INVARIANTS;
};

void Level::popBack()
{
    assert(!orders.empty() && "Cannot pop from empty level!");
    Order &order_to_remove = orders.back();
    volume -= order_to_remove.getOpenQuantity();
    orders.pop_back();
    LEVEL_CHECK_INVARIANTS;
}

void Level::deleteOrder(const Order &order)
{
    volume -= order.getOpenQuantity();
    orders.remove(order);
    LEVEL_CHECK_INVARIANTS;
}

void Level::reduceVolume(uint64_t amount)
{
    assert(volume >= amount && "Cannot reduce level volume by amount greater than its current volume!");
    volume -= amount;
    LEVEL_CHECK_INVARIANTS;
}

Order& Level::front()
{
    assert(!orders.empty() && "Level is empty!");
    return orders.front();
}

Order& Level::back()
{
    assert(!orders.empty() && "Level is empty!");
    return orders.back();
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
void Level::checkInvariants() const
{
    uint64_t actual_volume = 0;
    for (const auto &order : orders)
    {
        order.checkInvariants();
        assert(side == LevelSide::Ask ? order.isAsk() : order.isBid() && "Order side does not match level side!");
        if (order.isStop() || order.isStopLimit() || order.isTrailingStop() || order.isTrailingStopLimit())
            assert(order.getStopPrice() == price && "Order price does not match price of the level!");
        else
            assert(order.getPrice() == price && "Order price does not match price of the level!");
        actual_volume += order.getOpenQuantity();
    }

    if (actual_volume != volume)
    {
        std::cout << actual_volume << std::endl;
        std::cout << *this << std::endl;
    }
    assert(actual_volume == volume && "Level has incorrect volume!");
}
// LCOV_EXCL_STOP
