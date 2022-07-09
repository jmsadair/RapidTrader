#ifndef RAPID_TRADER_LEVEL_H
#define RAPID_TRADER_LEVEL_H
#include "order.h"

enum class LevelSide
{
    Bid,
    Ask
};

using namespace boost::intrusive;

class Level
{
public:
    Level(uint32_t price_, LevelSide side_, uint32_t symbol_id_);

    /**
     * @return the price associated with the level.
     */
    [[nodiscard]] inline uint32_t getPrice() const
    {
        return price;
    }

    /**
     * @return the total volume of the level.
     */
    [[nodiscard]] inline uint64_t getVolume() const
    {
        return volume;
    }

    /**
     * @return the side of the level - bid or ask.
     */
    [[nodiscard]] inline LevelSide getSide() const
    {
        return side;
    }

    /**
     * @return the symbol ID associated with the level.
     */
    [[nodiscard]] inline uint32_t getSymbolID() const
    {
        return symbol_id;
    }

    /**
     * @return true if the level is on the ask side and false otherwise.
     */
    [[nodiscard]] inline bool isAsk() const
    {
        return side == LevelSide::Ask;
    }

    /**
     * @return true if the order is on the bid side and false otherwise.
     */
    [[nodiscard]] inline bool isBid() const
    {
        return side == LevelSide::Bid;
    }

    /**
     * @return the number of orders in the level.
     */
    [[nodiscard]] inline size_t size() const
    {
        return orders.size();
    }

    /**
     * @return true if level is empty and false otherwise.
     */
    [[nodiscard]] inline bool empty() const
    {
        return orders.empty();
    }

    /**
     * @return the least recently inserted order in the level,
     *         require that the level is non-empty.
     */
    inline Order &front()
    {
        return orders.front();
    }

    /**
     * @return the most recently inserted order in the level,
     *         require that the level is non-empty.
     */
    inline Order &back()
    {
        return orders.back();
    }

    /**
     * Adds an order to the level.
     *
     * @param order the order to add, require that order is on the same side as the level,
     *              has the same price as the level, and has the same symbol ID as the level.
     */
    void addOrder(Order &order);

    /**
     * Removes the least recently inserted order from the level, require
     * that the level is non-empty.
     */
    void popFront();

    /**
     * Removes the most recently inserted order from the level, require that
     * the level is non-empty.
     */
    void popBack();

    /**
     * Deletes the order from the level.
     *
     * @param order the order to delete, require that the order
     *              is in the level.
     */
    inline void deleteOrder(const Order &order)
    {
        volume -= order.getOpenQuantity();
        orders.remove(order);
    }

    friend std::ostream &operator<<(std::ostream &os, const Level &level);

private:
    list<Order> orders;
    LevelSide side;
    uint32_t symbol_id;
    uint64_t volume;
    uint32_t price;
};
#endif // RAPID_TRADER_LEVEL_H
