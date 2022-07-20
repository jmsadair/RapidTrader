#ifndef RAPID_TRADER_ORDER_H
#define RAPID_TRADER_ORDER_H
#include <boost/intrusive/list.hpp>
#include <limits>
#include <array>

// Only check the order invariants in debug mode.
#ifndef NDEBUG
#    define ORDER_CHECK_INVARIANTS checkInvariants()
#else
#    define ORDER_CHECK_INVARIANTS
#endif

class MapOrderBook;
class Level;

// Represents the different actions of orders.
enum class OrderType
{
    Limit = 0,
    Market = 1,
    Stop = 2,
    StopLimit = 3,
    TrailingStop = 4,
    TrailingStopLimit = 5
};
static constexpr std::array order_type_to_string{"LIMIT", "MARKET", "STOP", "STOP LIMIT", "TRAILING STOP", "TRAILING STOP LIMIT"};

// Represents the different types of orders.
//  Good 'Till Cancelled: A good-til-canceled order will remain active until
//                       you decide to cancel it.
//  Fill Or Kill: A fill-or-kill order will be executed immediately in its entirety;
//                otherwise, it will be cancelled.
//  Immediate or Cancel: A immediate-or-cancel order will be executed immediately
//                       as fully as possible. Non-executed parts of the order are deleted
//                       without entry in the order book.
enum class OrderTimeInForce
{
    GTC = 0,
    FOK = 1,
    IOC = 2,
};
static constexpr std::array order_tof_to_string{"GTC", "FOK", "IOC"};

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
public:

    /**
     * Creates a market order on the ask side.
     *
     * @param order_id the ID of the order, require that order_id is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order, require that time_in_force is either
     *                      FOK or IOC.
     * @return a new market order.
     */
    static Order marketAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a market order on the bid side.
     *
     * @param order_id the ID of the order, require that order_id is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order, require that time_in_force is either
     *                      FOK or IOC.
     * @return a new market order.
     */
    static Order marketBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new limit order on the ask side.
     *
     * @param order_id the ID of the order, require that order_id is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param price the price of the order, require that price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order.
     * @return a new limit order.
     */
    static Order limitAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new limit order on the bid side.
     *
     * @param order_id the ID of the order, require that order_id is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param price the price of the order, require that price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order.
     * @return a new limit order.
     */
    static Order limitBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new stop market order on the ask side.
     *
     * @param order_id the ID of the order, require that ID is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param stop_price the stop price of the order, require that stop_price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order, require that time_in_force is either
     *                      FOK or IOC.
     * @return a new stop market order.
     */
    static Order stopAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new stop market order on the bid side.
     *
     * @param order_id the ID of the order, require that ID is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param stop_price the stop price of the order, require that stop_price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order, require that time_in_force is either
     *                      FOK or IOC.
     * @return a new stop market order.
     */
    static Order stopBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new stop limit order on the ask side.
     *
     * @param order_id the ID of the order, require that ID is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param price the price that the order, require that price is positive.
     * @param stop_price the stop price of the order, require that stop_price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order.
     * @return a new stop limit order.
     */
    static Order stopLimitAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * Creates a new stop limit order on the bid side.
     *
     * @param order_id the ID of the order, require that ID is positive.
     * @param symbol_id the symbol ID of the order, require that symbol ID is positive.
     * @param price the price that the order, require that price is positive.
     * @param stop_price the stop price of the order, require that stop_price is positive.
     * @param quantity the quantity of the order, require that quantity is positive.
     * @param time_in_force the time in force of the order.
     * @return a new stop limit order.
     */
    static Order stopLimitBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    static Order trailingStopAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    static Order trailingStopBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    static Order trailingStopLimitAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    static inline Order trailingStopLimitBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force);

    /**
     * @return the quantity of the order.
     */
    [[nodiscard]] uint64_t getQuantity() const
    {
        return quantity;
    }

    /**
     * @return the quantity of the order that has been executed.
     */
    [[nodiscard]] uint64_t getExecutedQuantity() const
    {
        return executed_quantity;
    }

    /**
     * @return the quantity of the order that remains to be executed.
     */
    [[nodiscard]] uint64_t getOpenQuantity() const
    {
        return open_quantity;
    }

    /**
     * @return the last executed quantity of the order if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] uint64_t getLastExecutedQuantity() const
    {
        return last_executed_quantity;
    }

    /**
     * @return the ID associated with the order.
     */
    [[nodiscard]] uint64_t getOrderID() const
    {
        return id;
    }

    /**
     * @return the price associated with the order.
     */
    [[nodiscard]] uint64_t getPrice() const
    {
        return price;
    }

    /**
     * @return the restart price associated with the order
     *         if applicable, otherwise zero.
     */
    [[nodiscard]] uint64_t getStopPrice() const
    {
        return stop_price;
    }

    /**
     * @return the side of the order - ask or bid.
     */
    [[nodiscard]] OrderSide getSide() const
    {
        return side;
    }

    /**
     * @return the time_in_force of the order - limit, market, stop, or restart limit.
     */
    [[nodiscard]] OrderType getType() const
    {
        return type;
    }

    /**
     * @return the time in force of the order - FOK, GTC, or IOC.
     */
    [[nodiscard]] OrderTimeInForce getTimeInForce() const
    {
        return time_in_force;
    }

    /**
     * @return the price at which the order was last executed if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] uint64_t getLastExecutedPrice() const
    {
        return last_executed_price;
    }

    /**
     * @return the symbol ID associated with the order.
     */
    [[nodiscard]] uint32_t getSymbolID() const
    {
        return symbol_id;
    }

    /**
     * @return true if order is on the ask side and false otherwise.
     */
    [[nodiscard]] bool isAsk() const
    {
        return side == OrderSide::Ask;
    }

    /**
     * @return true if order is on the bid side and false otherwise.
     */
    [[nodiscard]] bool isBid() const
    {
        return side == OrderSide::Bid;
    }

    /**
     * @return true if the order is a limit order and false otherwise.
     */
    [[nodiscard]] bool isLimit() const
    {
        return type == OrderType::Limit;
    }

    /**
     * @return true if the order is a market order and false otherwise.
     */
    [[nodiscard]] bool isMarket() const
    {
        return type == OrderType::Market;
    }

    /**
     * @return true if the order is a restart order and false otherwise.
     */
    [[nodiscard]] bool isStop() const
    {
        return type == OrderType::Stop;
    }

    /**
     * @return true if the order is a restart limit order and false otherwise.
     */
    [[nodiscard]] bool isStopLimit() const
    {
        return type == OrderType::StopLimit;
    }

    /**
     * @return true if the order is a trailing restart order and false otherwise.
     */
    [[nodiscard]] bool isTrailingStop() const
    {
        return type == OrderType::TrailingStop;
    }

    /**
     * @return true if the order is a trailing restart limit order and false otherwise.
     */
    [[nodiscard]] bool isTrailingStopLimit() const
    {
        return type == OrderType::TrailingStopLimit;
    }

    /**
     * @return true if the order is an IOC order and false otherwise.
     */
    [[nodiscard]] bool isIoc() const
    {
        return time_in_force == OrderTimeInForce::IOC;
    }

    /**
     * @return true if the order is a GTC order and false otherwise.
     */
    [[nodiscard]] bool isGtc() const
    {
        return time_in_force == OrderTimeInForce::GTC;
    }

    /**
     * @return true if the order is a FOK order and false otherwise.
     */
    [[nodiscard]] bool isFok() const
    {
        return time_in_force == OrderTimeInForce::FOK;
    }

    /**
     * @return true if the order is filled (i.e. the entire quantity of the order
     *         has been executed) and false otherwise.
     */
    [[nodiscard]] bool isFilled() const
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
    bool operator==(const Order &other) const
    {
        return id == other.id;
    }

    /**
     * Indicates whether two orders are not equal.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    bool operator!=(const Order &other) const
    {
        return !(*this == other);
    }

    friend std::ostream &operator<<(std::ostream &os, const Order &order);
    friend class MapOrderBook;
    friend class Level;

private:
    /**
     * A constructor for the Order.
     *
     * @param type_ the type of the order - Limit, Market, Stop, or Stop Limit.
     * @param side_ the side the order is on - ask or bid (i.e. buy or sell).
     * @param time_in_force_ the time in force of the order - FOK, GTC, or IOC.
     * @param symbol_id_ the symbol ID associated with the order.
     * @param price_ the price of the order, require that price_ is positive if the order is not a market
     *               order otherwise require that price_ is zero.
     * @param stop_price_ the stop price of the order, require that restart price is positive if a stop price
     *                    applies to the order type otherwise require that stop_price_ is zero.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param id_ the ID associated with the order, require that id_ is positive.
     */
    Order(OrderType type_, OrderSide side_, OrderTimeInForce time_in_force_, uint32_t symbol_id_, uint64_t price_, uint64_t stop_price_,
        uint64_t quantity_, uint64_t id_);

    /**
     * Executes the order.
     *
     * @param price_ the price at which to execute the order, require that price is positive.
     * @param quantity_ the quantity of the order to execute, require that quantity is positive.
     */
    void execute(uint32_t price_, uint64_t quantity_)
    {
        open_quantity -= quantity_;
        executed_quantity += quantity_;
        last_executed_price = price_;
        last_executed_quantity = quantity_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Set the price of the order.
     *
     * @param price_ the new price of that order, require that price
     *               is positive if the order is not a market order.
     */
    void setPrice(uint64_t price_)
    {
        price = price_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Set the stop price of the order, require that the order is a restart, stop limit,
     * or trailing restart order.
     *
     * @param stop_price_ the new price of that order, require that price_ is positive.
     */
    void setStopPrice(uint64_t stop_price_)
    {
        stop_price = stop_price_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Set the quantity of the order.
     *
     * @param quantity_ the new quantity of the order, require that quantity_ is positive.
     */
    void setQuantity(uint64_t quantity_)
    {
        quantity = std::min(quantity_, open_quantity);
        open_quantity -= quantity_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Set the ID of the order.
     *
     * @param id_ the new ID of the order.
     */
    void setOrderID(uint64_t id_)
    {
        id = id_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Set the time_in_force of the order.
     *
     * @param action_ the new time_in_force of the order, require that, if type_ is
     *                market or restart, the time_in_force of the order is FOK or IOC.
     */
    void setType(OrderType action_)
    {
        type = action_;
        ORDER_CHECK_INVARIANTS;
    }

    /**
     * Enforces the representation invariants of the order.
     *
     * @throws Error if the quantity of the order is not positive,
     *               the order is a market, stop, or trailing restart
     *               order and has time in force GTC, the order
     *               is a limit, restart limit, or trailing stop limit
     *               order and has a price that is not positive, the
     *               executed quantity of the order exceeds the quantity
     *               of the order, the last executed quantity of the
     *               order exceeds the quantity of the order, or the ID
     *               of the order is not positive.
     */
    void checkInvariants() const;

    OrderType type;
    OrderSide side;
    OrderTimeInForce time_in_force;
    uint32_t symbol_id;
    uint64_t price;
    uint64_t stop_price;
    uint64_t last_executed_price;
    uint64_t id;
    uint64_t quantity;
    uint64_t executed_quantity;
    uint64_t open_quantity;
    uint64_t last_executed_quantity;
};
#endif // RAPID_TRADER_ORDER_H
