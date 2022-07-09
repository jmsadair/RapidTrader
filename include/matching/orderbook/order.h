#ifndef RAPID_TRADER_ORDER_H
#define RAPID_TRADER_ORDER_H
#include <boost/intrusive/list.hpp>
#include <limits>
#include <array>

// Represents the different actions of orders.
enum class OrderAction
{
    Limit = 0,
    Market = 1,
    Stop = 2,
    StopLimit = 3
};
static constexpr std::array order_action_to_string{"LIMIT", "MARKET", "STOP", "STOP LIMIT"};

// Represents the different types of orders.
//  Good 'Till Cancelled: A good-til-canceled order will remain active until
//                       you decide to cancel it.
//  Fill Or Kill: A fill-or-kill order will be executed immediately in its entirety;
//                otherwise, it will be cancelled.
//  Immediate or Cancel: A immediate-or-cancel order will be executed immediately
//                       as fully as possible. Non-executed parts of the order are deleted
//                       without entry in the order book.
enum class OrderType
{
    GoodTillCancel = 0,
    FillOrKill = 1,
    ImmediateOrCancel = 2,
    AllOrNone = 3
};
static constexpr std::array order_type_to_string{"GTC", "FOK", "IOC", "AON"};

// Represents the side of the order.
enum class OrderSide
{
    Bid = 0,
    Ask = 1
};
static constexpr std::array order_side_to_string{"BID", "ASK"};

using namespace boost::intrusive;

struct Order : public list_base_hook<>
{
    /**
     * A constructor for the Order.
     *
     * @param action_ the action of the order, require that market orders are of type IOC or FOK.
     * @param side_ the side the order is on - ask or bid (i.e. buy or sell).
     * @param type_ the type of the order - FOK, GTC, or IOC.
     * @param symbol_id_ the symbol ID associated with the order.
     * @param price_ the price of the order, require that price_ is positive.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param id_ the ID associated with the order.
     */
    Order(OrderAction action_, OrderSide side_, OrderType type_, uint32_t symbol_id_, uint32_t price_, uint64_t quantity_, uint64_t id_);

    /**
     * Executes the order.
     *
     * @param price_ the price at which to execute the order, require that price is positive.
     * @param quantity_ the quantity of the order to execute, require that quantity is positive.
     */
    inline void execute(uint32_t price_, uint64_t quantity_)
    {
        open_quantity -= quantity_;
        executed_quantity += quantity_;
        last_executed_price = price_;
        last_executed_quantity = quantity_;
    }

    /**
     * @return the quantity of the order.
     */
    [[nodiscard]] inline uint64_t getQuantity() const
    {
        return quantity;
    }

    /**
     * @return the quantity of the order that has been executed.
     */
    [[nodiscard]] inline uint64_t getExecutedQuantity() const
    {
        return executed_quantity;
    }

    /**
     * @return the quantity of the order that remains to be executed.
     */
    [[nodiscard]] inline uint64_t getOpenQuantity() const
    {
        return open_quantity;
    }

    /**
     * @return the last executed quantity of the order if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] inline uint64_t getLastExecutedQuantity() const
    {
        return last_executed_quantity;
    }

    /**
     * @return the ID associated with the order.
     */
    [[nodiscard]] inline uint64_t getOrderID() const
    {
        return id;
    }

    /**
     * @return the price associated with the order.
     */
    [[nodiscard]] inline uint32_t getPrice() const
    {
        return price;
    }

    /**
     * @return the side of the order - ask or bid.
     */
    [[nodiscard]] inline OrderSide getSide() const
    {
        return side;
    }

    /**
     * @return the action of the order - limit or market.
     */
    [[nodiscard]] inline OrderAction getAction() const
    {
        return action;
    }

    /**
     * @return the type of the order - FOK, GTC, or IOC.
     */
    [[nodiscard]] inline OrderType getType() const
    {
        return type;
    }

    /**
     * @return the price at which the order was last executed if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] inline uint32_t getLastExecutedPrice() const
    {
        return last_executed_price;
    }

    /**
     * @return the symbol ID associated with the order.
     */
    [[nodiscard]] inline uint32_t getSymbolID() const
    {
        return symbol_id;
    }

    /**
     * Set the price of the order.
     *
     * @param price_ the new price of that order, require that price_ is positive.
     */
    inline void setPrice(uint32_t price_)
    {
        price = price_;
    }

    /**
     * Set the quantity of the order.
     *
     * @param quantity_ the new quantity of the order, require that quantity_ is positive.
     */
    inline void setQuantity(uint64_t quantity_)
    {
        quantity = quantity_;
        open_quantity = std::min(open_quantity, quantity);
    }

    /**
     * Set the ID of the order.
     *
     * @param id_ the new ID of the order.
     */
    inline void setOrderID(uint64_t id_)
    {
        id = id_;
    }

    /**
     * Set the action of the order.
     *
     * @param action_ the new action of the order, require that, if action_ is
     *                market or stop, the type of the order is FOK or IOC.
     */
    inline void setAction(OrderAction action_)
    {
        action = action_;
    }

    /**
     * Cancel a quantity of the order.
     *
     * @param quantity_ the quantity of the order to cancel, require that quantity_ is positive.
     */
    inline void cancel(uint64_t quantity_)
    {
        open_quantity = std::min(quantity_, quantity);
    }

    /**
     * @return true if order is on the ask side and false otherwise.
     */
    [[nodiscard]] inline bool isAsk() const
    {
        return side == OrderSide::Ask;
    }

    /**
     * @return true if order is on the bid side and false otherwise.
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
     * @return true if the order is a stop order and false otherwise.
     */
    [[nodiscard]] inline bool isStop() const
    {
        return action == OrderAction::Stop;
    }

    /**
     * @return true if the order is a stop limit order and false otherwise.
     */
    [[nodiscard]] inline bool isStopLimit() const
    {
        return action == OrderAction::StopLimit;
    }

    /**
     * @return true if the order is an IOC order and false otherwise.
     */
    [[nodiscard]] inline bool isIoc() const
    {
        return type == OrderType::ImmediateOrCancel;
    }

    /**
     * @return true if the order is a GTC order and false otherwise.
     */
    [[nodiscard]] inline bool isGtc() const
    {
        return type == OrderType::GoodTillCancel;
    }

    /**
     * @return true if the order is a FOK order and false otherwise.
     */
    [[nodiscard]] inline bool isFok() const
    {
        return type == OrderType::FillOrKill;
    }

    /**
     * @return true if the order is a AON order and false otherwise.
     */
    [[nodiscard]] inline bool isAon() const
    {
        return type == OrderType::AllOrNone;
    }

    /**
     * @return true if the order is filled (i.e. the entire quantity of the order
     *         has been executed) and false otherwise.
     */
    [[nodiscard]] inline bool isFilled() const
    {
        return open_quantity == 0;
    }

    /**
     * Indicates whether two orders are equal. Two orders are equal iff they
     * have the same order ID.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    inline bool operator==(const Order &other) const
    {
        return id == other.id;
    }

    /**
     * Indicates whether two orders are not equal.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    inline bool operator!=(const Order &other) const
    {
        return !(*this == other);
    }

    friend std::ostream &operator<<(std::ostream &os, const Order &order);

private:
    OrderAction action;
    OrderSide side;
    OrderType type;
    uint32_t symbol_id;
    uint32_t price;
    uint32_t last_executed_price;
    uint64_t id;
    uint64_t quantity;
    uint64_t executed_quantity;
    uint64_t open_quantity;
    uint64_t last_executed_quantity;
};
#endif // RAPID_TRADER_ORDER_H
