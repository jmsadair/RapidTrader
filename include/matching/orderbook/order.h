#ifndef RAPID_TRADER_ORDER_H
#define RAPID_TRADER_ORDER_H
#include <boost/intrusive/list.hpp>
#include <limits>
#include <array>

// Represents the different actions of orders.
enum class OrderAction
{
    Limit = 0,
    Market = 1
};
static constexpr std::array order_action_to_string{"LIMIT", "MARKET"};

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
static constexpr std::array order_type_to_string{"GTC", "FOK", "IOC"};

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
    Order(OrderAction action_, OrderSide side_, OrderType type_, uint32_t symbol_id_, uint32_t price_, uint64_t quantity_, uint64_t id_);

    inline void execute(uint32_t price_, uint64_t quantity_)
    {
        open_quantity -= quantity_;
        executed_quantity += quantity_;
        last_executed_price = price_;
        last_executed_quantity = quantity_;
    }

    [[nodiscard]] inline uint64_t getQuantity() const
    {
        return quantity;
    }

    [[nodiscard]] inline uint64_t getExecutedQuantity() const
    {
        return executed_quantity;
    }

    [[nodiscard]] inline uint64_t getOpenQuantity() const
    {
        return open_quantity;
    }

    [[nodiscard]] inline uint64_t getLastExecutedQuantity() const
    {
        return last_executed_quantity;
    }

    [[nodiscard]] inline uint64_t getOrderID() const
    {
        return id;
    }

    [[nodiscard]] inline uint32_t getPrice() const
    {
        return price;
    }

    [[nodiscard]] inline OrderSide getSide() const
    {
        return side;
    }

    [[nodiscard]] inline OrderAction getAction() const
    {
        return action;
    }

    [[nodiscard]] inline OrderType getType() const
    {
        return type;
    }

    [[nodiscard]] inline uint32_t getLastExecutedPrice() const
    {
        return last_executed_price;
    }

    [[nodiscard]] inline uint32_t getSymbolID() const
    {
        return symbol_id;
    }

    inline void setPrice(uint32_t price_)
    {
        price = price_;
    }

    inline void setQuantity(uint64_t quantity_)
    {
        quantity = quantity_;
        open_quantity = std::min(open_quantity, quantity);
    }

    inline void setOrderID(uint64_t id_)
    {
        id = id_;
    }

    inline void cancel(uint64_t quantity_)
    {
        open_quantity = std::min(quantity_, quantity);
    }

    [[nodiscard]] inline bool isAsk() const
    {
        return side == OrderSide::Ask;
    }

    [[nodiscard]] inline bool isBid() const
    {
        return side == OrderSide::Bid;
    }

    [[nodiscard]] inline bool isLimit() const
    {
        return action == OrderAction::Limit;
    }

    [[nodiscard]] inline bool isMarket() const
    {
        return action == OrderAction::Market;
    }

    [[nodiscard]] inline bool isIoc() const
    {
        return type == OrderType::ImmediateOrCancel;
    }

    [[nodiscard]] inline bool isGtc() const
    {
        return type == OrderType::GoodTillCancel;
    }

    [[nodiscard]] inline bool isFok() const
    {
        return type == OrderType::FillOrKill;
    }

    [[nodiscard]] inline bool isFilled() const
    {
        return open_quantity == 0;
    }

    inline bool operator==(const Order &other) const
    {
        return action == other.action && type == other.type && side == other.side && id == other.id && quantity == other.quantity &&
               executed_quantity == other.executed_quantity && price == other.price && open_quantity == other.open_quantity &&
               last_executed_quantity == other.last_executed_quantity && last_executed_price == other.last_executed_price &&
               symbol_id == other.symbol_id;
    }

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
