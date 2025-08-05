#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "../include/utils.h"

// Core order book functions
void initialize_price_levels(PriceLevel* levels, int max_levels);
void add_to_price_level(PriceLevel* level, Order* order);
void remove_from_price_level(PriceLevel* level, const char* order_id);
void sort_price_levels(PriceLevel* levels, int count, OrderSide side);
int find_price_level_index(PriceLevel* levels, int count, double price);
void execute_trade(OrderBook* book, Order* buy_order, Order* sell_order, int quantity);
void update_order_status(Order* order);
void cleanup_filled_orders(OrderBook* book);

#endif // ORDERBOOK_H