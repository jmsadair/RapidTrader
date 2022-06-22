#include "exchange.h"
#include "orderbook/price_level.h"

int main() {
    FastExchange::Exchange exchange {3, 3};
    exchange.start();
    auto api = exchange.getApi();
    Message::Command::AddOrderBook cmd1 {1};
    api.submitCommand(cmd1);
    Message::Command::AddOrderBook cmd2 {2};
    api.submitCommand(cmd2);
    Message::Command::PlaceOrder cmd3 {1, 1, 1, 200, 200, OrderAction::Limit, OrderSide::Ask, OrderType::GoodTillCancel};
    api.submitCommand(cmd3);
    Message::Command::PlaceOrder cmd4 {2, 2, 1, 200, 200, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd4);
    Message::Command::PlaceOrder cmd5 {3, 3, 2, 200, 250, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd5);
    Message::Command::PlaceOrder cmd6 {4, 4, 3, 100, 100, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd6);
}