#ifndef RAPID_TRADER_ORDER_H
#define RAPID_TRADER_ORDER_H
#include <boost/intrusive/list.hpp>
#include <limits>
#include <array>

class MapOrderBook;

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
    static inline Order marketAskOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Market orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Market, OrderSide::Ask, time_in_force, symbol_id, 0, 0, quantity, order_id};
    }

    static inline Order marketBidOrder(uint64_t order_id, uint32_t symbol_id, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Market orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Market, OrderSide::Bid, time_in_force, symbol_id, 0, 0, quantity, order_id};
    }

    static inline Order limitAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(price > 0 && "Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Limit, OrderSide::Ask, time_in_force, symbol_id, price, 0, quantity, order_id};
    }

    static inline Order limitBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(price > 0 && "Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Limit, OrderSide::Bid, time_in_force, symbol_id, price, 0, quantity, order_id};
    }

    static inline Order stopAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Stop, OrderSide::Ask, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
    }

    static inline Order stopBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Stop Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::Stop, OrderSide::Bid, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
    }

    static inline Order stopLimitAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(price > 0 && "Price must be positive!");
        assert(stop_price > 0 && "Stop Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::StopLimit, OrderSide::Ask, time_in_force, symbol_id, price, stop_price, quantity, order_id};
    }

    static inline Order stopLimitBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(price > 0 && "Price must be positive!");
        assert(stop_price > 0 && "Stop Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::StopLimit, OrderSide::Bid, time_in_force, symbol_id, price, stop_price, quantity, order_id};
    }

    static inline Order trailingStopAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::TrailingStop, OrderSide::Ask, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
    }

    static inline Order trailingStopBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Stop Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::TrailingStop, OrderSide::Bid, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
    }

    static inline Order trailingStopLimitAskOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::TrailingStopLimit, OrderSide::Ask, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
    }

    static inline Order trailingStopLimitBidOrder(
        uint64_t order_id, uint32_t symbol_id, uint64_t stop_price, uint64_t quantity, OrderTimeInForce time_in_force)
    {
        assert(time_in_force != OrderTimeInForce::GTC && "Stop orders cannot gave GTC time in force!");
        assert(order_id > 0 && "Order ID must be positive!");
        assert(symbol_id > 0 && "Symbol ID must be positive!");
        assert(stop_price > 0 && "Stop Price must be positive!");
        assert(quantity > 0 && "Quantity must be positive!");
        return Order{OrderType::TrailingStopLimit, OrderSide::Bid, time_in_force, symbol_id, 0, stop_price, quantity, order_id};
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
    [[nodiscard]] inline uint64_t getPrice() const
    {
        return price;
    }

    /**
     * @return the stop price associated with the order
     *         if applicable, otherwise zero.
     */
    [[nodiscard]] inline uint64_t getStopPrice() const
    {
        return stop_price;
    }

    /**
     * @return the side of the order - ask or bid.
     */
    [[nodiscard]] inline OrderSide getSide() const
    {
        return side;
    }

    /**
     * @return the time_in_force of the order - limit, market, stop, or stop limit.
     */
    [[nodiscard]] inline OrderType getType() const
    {
        return type;
    }

    /**
     * @return the time in force of the order - FOK, GTC, or IOC.
     */
    [[nodiscard]] inline OrderTimeInForce getTimeInForce() const
    {
        return time_in_force;
    }

    /**
     * @return the price at which the order was last executed if the order
     *         has been executed, otherwise zero.
     */
    [[nodiscard]] inline uint64_t getLastExecutedPrice() const
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
        return type == OrderType::Limit;
    }

    /**
     * @return true if the order is a market order and false otherwise.
     */
    [[nodiscard]] inline bool isMarket() const
    {
        return type == OrderType::Market;
    }

    /**
     * @return true if the order is a stop order and false otherwise.
     */
    [[nodiscard]] inline bool isStop() const
    {
        return type == OrderType::Stop;
    }

    /**
     * @return true if the order is a stop limit order and false otherwise.
     */
    [[nodiscard]] inline bool isStopLimit() const
    {
        return type == OrderType::StopLimit;
    }

    /**
     * @return true if the order is a trailing stop order and false otherwise.
     */
    [[nodiscard]] inline bool isTrailingStop() const
    {
        return type == OrderType::TrailingStop;
    }

    /**
     * @return true if the order is a trailing stop limit order and false otherwise.
     */
    [[nodiscard]] inline bool isTrailingStopLimit() const
    {
        return type == OrderType::TrailingStopLimit;
    }

    /**
     * @return true if the order is an IOC order and false otherwise.
     */
    [[nodiscard]] inline bool isIoc() const
    {
        return time_in_force == OrderTimeInForce::IOC;
    }

    /**
     * @return true if the order is a GTC order and false otherwise.
     */
    [[nodiscard]] inline bool isGtc() const
    {
        return time_in_force == OrderTimeInForce::GTC;
    }

    /**
     * @return true if the order is a FOK order and false otherwise.
     */
    [[nodiscard]] inline bool isFok() const
    {
        return time_in_force == OrderTimeInForce::FOK;
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
    friend class MapOrderBook;

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
     * @param stop_price_ the stop price of the order, require that stop price is positive if a stop price
     *                    applies to the order type otherwise require that stop_price_ is zero.
     * @param quantity_ the quantity of the order, require that quantity_ is positive.
     * @param id_ the ID associated with the order.
     */
    Order(OrderType type_, OrderSide side_, OrderTimeInForce time_in_force_, uint32_t symbol_id_, uint64_t price_, uint64_t stop_price_,
        uint64_t quantity_, uint64_t id_);

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
     * Set the price of the order.
     *
     * @param price_ the new price of that order, require that price
     *               is positive if the order is not a market order.
     */
    inline void setPrice(uint64_t price_)
    {
        price = price_;
    }

    /**
     * Set the stop price of the order, require that the order is a stop, stop limit,
     * or trailing stop order.
     *
     * @param stop_price_ the new price of that order, require that price_ is positive.
     */
    inline void setStopPrice(uint64_t stop_price_)
    {
        stop_price = stop_price_;
    }

    /**
     * Set the quantity of the order.
     *
     * @param quantity_ the new quantity of the order, require that quantity_ is positive.
     */
    inline void setQuantity(uint64_t quantity_)
    {
        quantity = std::min(quantity_, open_quantity);
        open_quantity -= quantity_;
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
     * Set the time_in_force of the order.
     *
     * @param action_ the new time_in_force of the order, require that, if type_ is
     *                market or stop, the time_in_force of the order is FOK or IOC.
     */
    inline void setType(OrderType action_)
    {
        type = action_;
    }

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
