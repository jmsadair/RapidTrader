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
#include "types.h"

using namespace boost::intrusive;

/**
 * A mutable order ADT.
 */
struct Order : public list_base_hook<> {
    const OrderAction action;
    const OrderSide side;
    const OrderType type;
    const UserID user_id;
    const OrderID id;
    const Price price;
    const Quantity quantity;
    // The remaining quantity that must be filled.
    Quantity quantity_to_fill;
    OrderStatus status;


    /**
     * A constructor for the order ADT.
     * TODO: When is an order rejected?
     *
     * @param action_ the action of the order - limit or market.
     * @param side_ the side of the order - ask or bid.
     * @param type_ type of the order - GTC, IOC, FOK.
     * @param quantity_ the quantity of the order, require that quantity is positive.
     * @param price_ the price of the order, require that price is positive.
     * @param id_ a unique ID associated with the order.
     * @param user_id_ a unique ID associated with the user placing the order.
     * @param status_ the status of the of the order.
     * @throws Error if price is negative or if quantity is not positive.
     */
    Order(OrderAction action_, OrderSide side_, OrderType type_, Quantity quantity_, Price price_, OrderID id_,
          UserID user_id_, OrderStatus status_ = OrderStatus::Accepted)
            : action(action_), side(side_), type(type_), quantity(quantity_),
              price(price_), id(id_), user_id(user_id_), quantity_to_fill(quantity_), status(status_)
    {
        if (quantity == 0)
            throw std::invalid_argument("quantity must be positive!");
        if (price == 0)
            throw std::invalid_argument("price must be non-negative!");
    }

    /**
     * A constructor for the Order ADT.
     *
     * @param action_ the action of the order - limit or market.
     * @param side_ the side of the order - ask or bid.
     * @param type_ type of the order - GTC, IOC, FOK.
     * @param quantity_ the quantity of the order, require that quantity is positive.
     * @param quantity_to_fill_ the quantity of the order that remains to be filled, require that quantity_to_fill is
     *                          positive.
     * @param price_ the price of the order, require that price is positive.
     * @param id_ a unique ID associated with the order.
     * @param user_id_ a unique ID associated with the user placing the order.
     * @param status_ the status of the order.
     */
    Order(OrderAction action_, OrderSide side_, OrderType type_, Quantity quantity_, Quantity quantity_to_fill_,
          Price price_, OrderID id_, UserID user_id_, OrderStatus status_ = OrderStatus::Accepted)
            : action(action_), side(side_), type(type_), quantity(quantity_), quantity_to_fill(quantity_to_fill_),
              price(price_), id(id_), user_id(user_id_), status(status_)
    {
        if (quantity == 0)
            throw std::invalid_argument("quantity must be positive!");
        if (price == 0)
            throw std::invalid_argument("price must be non-negative!");
    }

    /**
     * Indicates whether two orders are equal. Two orders are equal if and only if they have
     * the same status, the same action, the same side, the same type, the same ID, the same user ID, the
     * same quantity, the same quantity remaining to be filled, and the same price.
     *
     * @param other another order.
     * @return true if the orders are equal and false otherwise.
     */
    bool operator==(const Order &other) const {
        return status == other.status &&
               action == other.action &&
               type == other.type &&
               side == other.side &&
               id == other.id &&
               user_id == other.user_id &&
               quantity == other.quantity &&
               quantity_to_fill == other.quantity_to_fill &&
               price == other.price;
    }

    /**
     * Indicates whether two orders are not equal. Two orders are equal if and only if they have
     * the same status, the same action, the same side, the same type, the same ID, the same user ID, the
     * same quantity, the same quantity remaining to be filled, and the same price.
     *
     * @param other another order.
     * @return true if the orders are not equal and false otherwise.
     */
    bool operator!=(const Order &other) const {
        return !(*this == other);
    }

    /**
     * @return the string representation of an order.
     */
    [[nodiscard]] std::string toString() const {
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
               "\nOrder Side: " + order_side +
               "\nOrder Action: " + order_action +
               "\nOrder Status: " + order_status +
               "\nOrder Type: " + order_type +
               "\nOrder Price: " + order_price +
               "\nOrder Quantity: " + order_quantity + "\n";
    }
};
#endif //FAST_EXCHANGE_ORDER_H
