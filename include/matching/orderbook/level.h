#ifndef RAPID_TRADER_LEVEL_H
#define RAPID_TRADER_LEVEL_H
#include "order.h"

// Only check the order level invariants in debug mode.
#ifndef NDEBUG
#    define LEVEL_CHECK_INVARIANTS checkInvariants()
#else
#    define LEVEL_CHECK_INVARIANTS
#endif

enum class LevelSide
{
    Bid,
    Ask
};

using namespace boost::intrusive;

class Level
{
public:
    Level(uint64_t price_, LevelSide side_, uint32_t symbol_id_);

    /**
     * @return the orders in the level.
     */
    [[nodiscard]] inline const list<Order> &getOrders() const
    {
        return orders;
    }

    /**
     * @return the orders in the level.
     */
    [[nodiscard]] inline list<Order> &getOrders()
    {
        return orders;
    }

    /**
     * @return the price associated with the level.
     */
    [[nodiscard]] inline uint64_t getPrice() const
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
        assert(!orders.empty() && "Level is empty!");
        return orders.front();
    }

    /**
     * @return the most recently inserted order in the level,
     *         require that the level is non-empty.
     */
    inline Order &back()
    {
        assert(!orders.empty() && "Level is empty!");
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
    /**
     * Enforces the representation invariants of the level.
     *
     * @throws Error if any order in the level has a price that does not
     *                match the price of the level, if any order in the level
     *                is on a different side than the level, or if the sum of the
     *                open quantities of the orders in the level do not match the
     *                volume of the level.
     */
    void checkInvariants() const;

    list<Order> orders;
    LevelSide side;
    uint32_t symbol_id;
    uint64_t volume;
    uint64_t price;
};
#endif // RAPID_TRADER_LEVEL_H
