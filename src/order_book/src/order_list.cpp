#include <algorithm>
#include "order_list.h"

OrderList::OrderList(Order& order) {
    order_list.push_back(order);
}

void OrderList::addOrder(Order& order) {
    order_list.push_back(order);
}

void OrderList::removeOrder(const Order &order) {
    auto order_it = order_list.iterator_to(order);
    // Require that order is in list.
    assert(order_it != order_list.end());
    order_list.erase(order_it);
}
bool OrderList::isEmpty() {
    return order_list.empty();
}

size_t OrderList::size() {
    return order_list.size();
}

bool OrderList::hasOrder(const Order& order) {
    return std::find(order_list.begin(), order_list.end(), order) != order_list.end();
}
