#include "exchange.h"

int main() {
    FastExchange::Exchange exchange {1};
    FastExchange::ExchangeApi& api = exchange.getApi();
    Message::Command::AddOrderBook cmd1 {1};
    api.submitCommand(cmd1);
    Message::Command::AddOrderBook cmd2 {2};
    api.submitCommand(cmd2);
    Message::Command::AddOrderBook cmd3 {3};
    api.submitCommand(cmd3);
    Message::Command::AddOrderBook cmd4 {4};
    api.submitCommand(cmd4);
    Message::Command::AddOrderBook cmd5 {5};
    api.submitCommand(cmd5);
    Message::Command::PlaceOrder cmd6 {1, 1, 1, 200, 200, OrderAction::Limit, OrderSide::Ask, OrderType::GoodTillCancel};
    api.submitCommand(cmd6);
    Message::Command::PlaceOrder cmd7 {2, 2, 1, 200, 200, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd7);
}