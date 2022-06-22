#ifndef FAST_EXCHANGE_ORDER_H
#define FAST_EXCHANGE_ORDER_H
#include <iomanip>
#include <boost/intrusive/list.hpp>
#include "types.h"

using namespace boost::intrusive;

/**
 * A mutable order ADT.
 */
struct Order : public list_base_hook<>
{
    OrderAction action;
    OrderSide side;
    OrderType type;
    uint64_t quantity;
    uint32_t price;
    uint64_t id;
    uint64_t user_id;
    uint32_t symbol_id;
    uint64_t quantity_executed;
    Order() noexcept = default;
    /**
     * A constructor for the order ADT.
     *
     * @param action_ the action of the order - limit or market.
     * @param side_ the side od the order - ask or bid.
     * @param type_ the type of the order - GTC, IOC, FOK.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param price_ the price of the order, require that price_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     * @param user_id_ the user ID associated with the order, require that user_id_ is positive.
     * @param symbol_id_ the symbol ID associated with the order, require that symbol_id_ is positive.
     * @param status_ the status of the order - accepted, partially filled, filled, or rejected.
     */
    inline Order(OrderAction action_, OrderSide side_, OrderType type_, uint64_t quantity_, uint32_t price_, uint64_t id_,
        uint64_t user_id_, uint32_t symbol_id_)
        : action(action_)
        , side(side_)
        , type(type_)
        , quantity(quantity_)
        , price(price_)
        , id(id_)
        , user_id(user_id_)
        , symbol_id(symbol_id_)
        , quantity_executed(0)
    {}

    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;

    /**
     * @return the quantity of the order that remains to be executed.
     */
    [[nodiscard]] inline uint64_t executableQuantity() const
    {
        return quantity - quantity_executed;
    }

    /**
     * @return true if the order is on the ask side and false otherwise.
     */
    [[nodiscard]] inline bool isAsk() const
    {
        return side == OrderSide::Ask;
    }

    /**
     * @return true if the order is on the bid side and false otherwise.
     */
    [[nodiscard]] inline bool isBid() const
    {
        return side == OrderSide::Bid;
    }

    /**
     * @return true if the order is a limit order and false otherwise.
     */
    [[nodiscard]] inline bool isLimit() const
    {
        return action == OrderAction::Limit;
    }

    /**
     * @return true if the order is a market order and false otherwise.
     */
    [[nodiscard]] inline bool isMarket() const
    {
        return action == OrderAction::Market;
    }

    /**
     * @return true if the order is of type IOC and false otherwise.
     */
    [[nodiscard]] inline bool isIoc() const
    {
        return type == OrderType::ImmediateOrCancel;
    }

    /**
     * @return true if the order is of type GTC and false otherwise.
     */
    [[nodiscard]] inline bool isGtc() const
    {
        return type == OrderType::GoodTillCancel;
    }

    /**
     * @return true if the complete quantity of the order has been executed and false otherwise.
     */
    [[nodiscard]] inline bool isFilled() const
    {
        return quantity_executed == quantity;
    }

    /**
     * Indicates whether two orders are equal. Two orders are equal if and only if they have
     * the same status, the same action, the same side, the same type, the same ID, the same user ID, the
     * same quantity, the same quantity remaining to be filled, and the same price.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    inline bool operator==(const Order &other) const
    {
        return action == other.action && type == other.type && side == other.side && id == other.id && user_id == other.user_id &&
               quantity == other.quantity && quantity_executed == other.quantity_executed && price == other.price;
    }

    /**
     * Indicates whether two orders are not equal. Two orders are equal if and only if they have
     * the same status, the same action, the same side, the same type, the same ID, the same user ID, the
     * same quantity, the same quantity remaining to be filled, and the same price.
     *
     * @param other another order.
     * @return true if the orders are not equal and false otherwise.
     */
    inline bool operator!=(const Order &other) const
    {
        return !(*this == other);
    }

    /**
     * Creates an order on the ask side with a limit action.
     *
     * @param type_ the type of the order - IOC, FOK, or GTC.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param price_ the price of the order, require that price_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     * @param user_id_ the user ID associated with the order, require that user_id_ is positive.
     * @param symbol_id_ the symbol ID associated with the order, require that symbol_id_ is positive.
     * @return a new order.
     */
    static inline Order askLimit(OrderType type_, uint64_t quantity_, uint32_t price_, uint64_t id_, uint64_t user_id_, uint32_t symbol_id_)
    {
        return {OrderAction::Limit, OrderSide::Ask, type_, quantity_, price_, id_, user_id_, symbol_id_};
    }

    /**
     * Creates an order on the bid side with a limit action.
     *
     * @param type_ the type of the order - IOC, FOK, or GTC.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param price_ the price of the order, require that price_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     * @param user_id_ the user ID associated with the order, require that user_id_ is positive.
     * @param symbol_id_ the symbol ID associated with the order, require that symbol_id_ is positive.
     * @return a new order.
     */
    static inline Order bidLimit(OrderType type_, uint64_t quantity_, uint32_t price_, uint64_t id_, uint64_t user_id_, uint32_t symbol_id_)
    {
        return {OrderAction::Limit, OrderSide::Bid, type_, quantity_, price_, id_, user_id_, symbol_id_};
    }

    /**
     * Creates an order on the ask side with a market action.
     *
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param price_ the price of the order, require that price_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     * @param user_id_ the user ID associated with the order, require that user_id_ is positive.
     * @param symbol_id_ the symbol ID associated with the order, require that symbol_id_ is positive.
     * @return a new order.
     */
    static inline Order askMarket(uint64_t quantity_, uint32_t price_, uint64_t id_, uint64_t user_id_, uint32_t symbol_id_)
    {
        return {OrderAction::Market, OrderSide::Ask, OrderType::ImmediateOrCancel, quantity_, price_, id_, user_id_, symbol_id_};
    }

    /**
     * Creates an order on the bid side with a market action.
     *
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param price_ the price of the order, require that price_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     * @param user_id_ the user ID associated with the order, require that user_id_ is positive.
     * @param symbol_id_ the symbol ID associated with the order, require that symbol_id_ is positive.
     * @return a new order.
     */
    static inline Order bidMarket(uint64_t quantity_, uint32_t price_, uint64_t id_, uint64_t user_id_, uint32_t symbol_id_)
    {
        return {OrderAction::Limit, OrderSide::Bid, OrderType::ImmediateOrCancel, quantity_, price_, id_, user_id_, symbol_id_};
    }
};

#endif // FAST_EXCHANGE_ORDER_H
