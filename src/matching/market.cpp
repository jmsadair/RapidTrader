#include "market.h"
#include "notification.h"
#include "log.h"
#include "map_orderbook.h"

namespace RapidTrader::Matching {
Market::Market(Messaging::Sender outgoing_messenger_)
    : outgoing_messenger(outgoing_messenger_)
{}

void Market::addOrderbook(uint32_t symbol_id)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id >= symbol_to_book.size() || !symbol_to_book[symbol_id] && "Orderbook already exists!");
    if (symbol_id >= symbol_to_book.size())
        symbol_to_book.resize(symbol_id + 1);
    symbol_to_book[symbol_id] = std::make_unique<MapOrderBook>(symbol_id);
    outgoing_messenger.send(AddedOrderBook{symbol_id});
}

void Market::addSymbol(uint32_t symbol_id, std::string symbol_name)
{
    assert(id_to_symbol.find(symbol_id) == id_to_symbol.end() && "Symbol already exists!");
    outgoing_messenger.send(AddedSymbol{symbol_id, symbol_name});
    id_to_symbol.insert({symbol_id, std::move(symbol_name)});
}
void Market::deleteOrderbook(uint32_t symbol_id)
{
    assert(symbol_id < symbol_to_book.size() && !symbol_to_book[symbol_id] && "Orderbook does not exist!");
    symbol_to_book[symbol_id] = nullptr;
    outgoing_messenger.send(DeletedOrderBook{symbol_id});
}
void Market::deleteSymbol(uint32_t symbol_id)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    outgoing_messenger.send(DeletedSymbol{symbol_id, id_to_symbol[symbol_id]});
    symbol_to_book[symbol_id] = nullptr;
    id_to_symbol.erase(symbol_id);
}
void Market::deleteOrder(uint32_t symbol_id, uint64_t order_id)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id] && "Orderbook does not exist!");
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    assert(book->hasOrder(order_id) && "Order does not exist !");
    outgoing_messenger.send(DeletedOrder{book->getOrder(order_id)});
    book->deleteOrder(order_id);
}

void Market::addOrder(const Order &order)
{
    assert(id_to_symbol.find(order.getSymbolID()) != id_to_symbol.end() && "Symbol does not exist!");
    assert(order.getSymbolID() < symbol_to_book.size() && symbol_to_book[order.getSymbolID()] && "Orderbook does not exist!");
    // Get orderbook that corresponds to order symbol ID.
    OrderBook *book = symbol_to_book[order.getSymbolID()].get();
    // Make sure order does not already exist.
    assert(!book->hasOrder(order.getOrderID()) && "Order already exists!");
    outgoing_messenger.send(AddedOrder{order});
    order.isMarket() ? addMarketOrder(book, order) : addLimitOrder(book, order);
}
void Market::addLimitOrder(OrderBook *book, Order order)
{
    // Matched orders.
    std::queue<Order> processed_orders;
    // Order is Fill or Kill and cannot be filled.
    if (order.isFok() && !book->canProcess(order))
    {
        outgoing_messenger.send(DeletedOrder{order});
        return;
    }
    // Match the order.
    book->processMatching(order, processed_orders);
    while (!processed_orders.empty())
    {
        auto processed_order = processed_orders.front();
        outgoing_messenger.send(ExecutedOrder{processed_order});
        if (processed_order.isFilled() && processed_order.getOrderID() != order.getOrderID())
            outgoing_messenger.send(DeletedOrder{processed_order});
        processed_orders.pop();
    }
    // If the order is GTC and not filled, add it the book.
    if (order.isGtc() && !order.isFilled())
        book->addOrder(order);
    else
        outgoing_messenger.send(DeletedOrder{order});
}
void Market::addMarketOrder(OrderBook *book, Order order)
{
    // Matched orders.
    std::queue<Order> processed_orders;
    order.setPrice(order.isAsk() ? std::numeric_limits<uint32_t>::min() : std::numeric_limits<uint32_t>::max());
    // Match the order.
    book->processMatching(order, processed_orders);
    while (!processed_orders.empty())
    {
        auto processed_order = processed_orders.front();
        outgoing_messenger.send(ExecutedOrder{processed_order});
        if (processed_order.isFilled() && processed_order.getOrderID() != order.getOrderID())
            outgoing_messenger.send(DeletedOrder{processed_order});
        processed_orders.pop();
    }
    outgoing_messenger.send(DeletedOrder{order});
}
void Market::cancelOrder(uint32_t symbol_id, uint64_t order_id, uint64_t cancelled_quantity)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id] && "Orderbook does not exist!");
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    assert(book->hasOrder(order_id) && "Order does not exist!");
    bool delete_order = book->cancelOrder(order_id, cancelled_quantity);
    outgoing_messenger.send(UpdatedOrder{book->getOrder(order_id)});
    // If cancelling the requested quantity of shares mutates the order such that is has no open quantity,
    // then delete the order.
    if (delete_order)
    {
        outgoing_messenger.send(DeletedOrder{book->getOrder(order_id)});
        book->deleteOrder(order_id);
    }
}
void Market::replaceOrder(uint32_t symbol_id, uint64_t order_id, uint64_t new_order_id, uint32_t new_price)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id] && "Orderbook does not exist!");
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    assert(book->hasOrder(order_id) && "Order does not exist!");
    // Find the order to replace.
    auto order_to_replace = book->getOrder(order_id);
    // Make a copy of the order with the modified ID and price, and reinsert into the book.
    Order new_order = order_to_replace;
    new_order.setOrderID(new_order_id);
    new_order.setPrice(new_price);
    outgoing_messenger.send(DeletedOrder{order_to_replace});
    outgoing_messenger.send(AddedOrder{new_order});
    book->deleteOrder(order_id);
    addLimitOrder(book, new_order);
}
void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity, uint32_t price)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id] && "Orderbook does not exist!");
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    assert(book->hasOrder(order_id) && "Order does not exist!");
    bool delete_order = book->executeOrder(order_id, quantity, price);
    // Get the executed order.
    auto order_executed = book->getOrder(order_id);
    outgoing_messenger.send(ExecutedOrder(order_executed));
    // If executing the requested number of
    if (delete_order)
    {
        outgoing_messenger.send(DeletedOrder{book->getOrder(order_id)});
        book->deleteOrder(order_id);
    }
}
void Market::executeOrder(uint32_t symbol_id, uint64_t order_id, uint64_t quantity)
{
    assert(id_to_symbol.find(symbol_id) != id_to_symbol.end() && "Symbol does not exist!");
    assert(symbol_id < symbol_to_book.size() && symbol_to_book[symbol_id] && "Orderbook does not exist!");
    OrderBook *book = symbol_to_book[symbol_id].get();
    // Check that the orderbook has the order.
    assert(book->hasOrder(order_id) && "Order does not exist!");
    bool delete_order = book->executeOrder(order_id, quantity);
    // Get the executed order.
    auto order_executed = book->getOrder(order_id);
    outgoing_messenger.send(ExecutedOrder(order_executed));
    // If executing the requested number of
    if (delete_order)
    {
        outgoing_messenger.send(DeletedOrder{book->getOrder(order_id)});
        book->deleteOrder(order_id);
    }
}
} // namespace RapidTrader::Matching