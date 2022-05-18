#ifndef FAST_EXCHANGE_ORDER_H
#define FAST_EXCHANGE_ORDER_H
#include <chrono>
#include <string>
#include <utility>
#include <array>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <boost/intrusive/list.hpp>

// Represents the different actions of orders.
enum class OrderAction {Limit = 0, Market = 1};
constexpr std::array order_action_to_string {"LIMIT", "MARKET"};

//Represents the different types of orders.
// Good 'Til Cancelled: A good-til-canceled order will remain active until
//                      you decide to cancel it.
// Fill Or Kill: A fill-or-kill order will be executed immediately in its entirety;
//             otherwise, it will be cancelled.
// Immediate or Cancel: A immediate-or-cancel order will be executed immediately
//                      as fully as possible. Non-executed parts of the order are deleted
//                      without entry in the order book.
enum class OrderType {GoodTillCancel = 0, FillOrKill = 1, ImmediateOrCancel = 2};
constexpr std::array order_type_to_string {"GTC", "FOK", "IOC"};

// Represents the side of the order.
enum class OrderSide {Bid = 0, Ask = 1};
constexpr std::array order_side_to_string {"BID", "ASK"};

// Represents the status of an order.
enum class OrderStatus {Accepted = 0, Rejected = 1, PartiallyFilled = 1, Filled = 2, Cancelled = 3};
constexpr std::array order_status_to_string {"ACCEPTED", "REJECTED", "PARTIALLY FILLED", "FILLED", "CANCELLED"};

using namespace boost::intrusive;

/**
 * An mutable order ADT.
 */
struct Order: public list_base_hook<> {
    const OrderAction action;
    const OrderSide side;
    const OrderType type;
    const uint64_t user_id;
    const uint64_t id;
    const std::string symbol;
    const std::chrono::time_point<std::chrono::system_clock> time;
    const float price;
    OrderStatus status;
    uint64_t quantity;

    /**
     * A constructor for the order ADT.
     * TODO: When is an order rejected?
     *
     * @param order_action the action of the order - limit or market.
     * @param order_side the side of the order - ask or bid.
     * @param order_type the type of the order - GTC, IOC, FOK.
     * @param order_quantity the quantity of the order, require that quantity is positive.
     * @param order_price the price of the order, require that price is non-negative.
     * @param order_id a unique ID associated with the order.
     * @param order_user_id a unique ID associated with the user placing the order.
     * @throws Error if price is negative or if quantity is not positive.
     */
    Order(OrderAction order_action, OrderSide order_side, OrderType order_type, uint64_t order_quantity, float order_price,
          uint64_t order_id, uint64_t order_user_id)
    : action(order_action), side(order_side), type(order_type), status(OrderStatus::Accepted), quantity(order_quantity),
      price(order_price), id(order_id), user_id(order_user_id), time(std::chrono::system_clock::now())
    {
        if (quantity == 0)
            throw std::invalid_argument("quantity must be positive!");
        if (price < 0)
            throw std::invalid_argument("price must be non-negative!");
    }

    /**
     * Indicates whether two orders are equal. Two orders are equal if and only if they have
     * the same status, the same side, the same ID, and the same price (within epsilon).
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    bool operator==(const Order& other) const {
        return status == other.status &&
               action == other.action &&
               type == other.type &&
               side == other.side &&
               id == other.id &&
               user_id == other.user_id &&
               quantity == other.quantity &&
               (std::fabs(price - other.price) <=
                std::numeric_limits<float>::epsilon() * std::fmax(std::fabs(price), std::fabs(other.price)));
    }

    bool operator!=(const Order& other) const {
        return !(*this == other);
    }
    /**
     * @return the string representation of an order.
     */
    std::string toString() {
        auto order_time = std::chrono::system_clock::to_time_t(time);
        std::string time_string = std::ctime(&order_time);
        std::string order_action = order_action_to_string[static_cast<std::underlying_type<OrderStatus>::type>(action)];
        std::string order_side = order_side_to_string[static_cast<std::underlying_type<OrderStatus>::type>(side)];
        std::string order_status = order_status_to_string[static_cast<std::underlying_type<OrderStatus>::type>(status)];
        std::string order_type = order_type_to_string[static_cast<std::underlying_type<OrderStatus>::type>(type)];
        std::string order_price = std::to_string(price);
        std::string order_quantity = std::to_string(quantity);
        std::string order_id = std::to_string(id);
        std::string order_user_id = std::to_string(user_id);

        return "Order ID: " + order_id +
                "\nUser ID: " + order_user_id +
                "\nOrder Time: " + time_string +
                "\nOrder Side: " + order_side +
                "\nOrder Action: " + order_action +
                "\nOrder Status: " + order_status +
                "\nOrder Type: " + order_type +
                "\nOrder Price: " + order_price +
                "\nOrder Quantity: " + order_quantity + "\n";
    }
};
#endif //FAST_EXCHANGE_ORDER_H
