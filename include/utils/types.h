#ifndef FAST_EXCHANGE_TYPES_H
#define FAST_EXCHANGE_TYPES_H
#include <array>
#include <string>
// Represents the different actions of orders.
enum class OrderAction
{
    Limit = 0,
    Market = 1
};
constexpr std::array order_action_to_string{"LIMIT", "MARKET"};

// Represents the different types of orders.
//  Good 'Til Cancelled: A good-til-canceled order will remain active until
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
    ImmediateOrCancel = 2
};
constexpr std::array order_type_to_string{"GTC", "FOK", "IOC"};

// Represents the side of the order.
enum class OrderSide
{
    Bid = 0,
    Ask = 1
};
constexpr std::array order_side_to_string{"BID", "ASK"};
#endif // FAST_EXCHANGE_TYPES_H
