#include "exchange.h"
#include "command.h"

int main()
{
    FastExchange::Exchange exchange;
    auto api = exchange.getApi();
    Command::AddOrderBook cmd1{1};
    api.submitCommand(cmd1);
    Command::AddOrderBook cmd2{2};
    api.submitCommand(cmd2);
    Command::PlaceOrder cmd3{1, 1, 1, 200, 200, OrderAction::Limit, OrderSide::Ask, OrderType::GoodTillCancel};
    api.submitCommand(cmd3);
    Command::PlaceOrder cmd4{2, 2, 1, 200, 200, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd4);
    Command::PlaceOrder cmd5{3, 3, 2, 200, 250, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd5);
    Command::PlaceOrder cmd6{4, 4, 2, 100, 100, OrderAction::Limit, OrderSide::Bid, OrderType::GoodTillCancel};
    api.submitCommand(cmd6);
}