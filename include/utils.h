#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_ID_LENGTH 16
#define MAX_SYMBOL_LENGTH 8
#define MAX_ORDERS 10000
#define MAX_PRICE_LEVELS 100

//Order types

typedef enum {
    BUY,
    SELL
} OrderSide;

//Order status
typedef enum {
    OPEN,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED
} OrderStatus;

//Order Struct

typedef struct {
    char id[MAX_ID_LENGTH];
    char symbol[MAX_SYMBOL_LENGTH];
    OrderSide side;
    double price;
    int quantity;
    int filled_quantity;
    time_t timestamp;
    OrderStatus status;
} Order;

//Price level struct
typedef struct {
    double price;
    int total_quantity;
    struct Order* orders;
    int order_count;
} PriceLevel;

//Order book struct
typedef struct {
    char symbol[MAX_SYMBOL_LENGTH];
    PriceLevel* buy_levels;
    int buy_level_count;
    PriceLevel* sell_levels;
    int sell_level_count;
    struct Order* all_orders;
    int order_count;
} OrderBook;

//Functions

OrderBook* create_order_book(const char* symbol);
void free_order_book(OrderBook* book);
void add_order(OrderBook* book, Order* order);
void cancel_order(OrderBook* book, const char* order_id);
void modify_order(OrderBook* book, const char* order_id, int new_quantity, double new_price);
void match_orders(OrderBook* book);
void print_order_book(const OrderBook* book);
void print_order(const Order* order);
int load_orders_from_csv(OrderBook* book, const char* filename);
Order* find_order_by_id(OrderBook* book, const char* order_id);
void process_user_input(OrderBook* book);
void display_help();

#endif //UTILS_H
