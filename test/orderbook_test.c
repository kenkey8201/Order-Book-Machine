#include "../include/utils.h"
#include "../src/orderbook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void test_create_order_book() {
    printf("Testing create_order_book... ");
    
    OrderBook* book = create_order_book("TEST");
    assert(book != NULL);
    assert(strcmp(book->symbol, "TEST") == 0);
    assert(book->buy_level_count == 0);
    assert(book->sell_level_count == 0);
    assert(book->order_count == 0);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_add_order() {
    printf("Testing add_order... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add a buy order
    Order buy_order;
    strcpy(buy_order.id, "B1");
    strcpy(buy_order.symbol, "TEST");
    buy_order.side = BUY;
    buy_order.price = 100.0;
    buy_order.quantity = 10;
    
    add_order(book, &buy_order);
    assert(book->buy_level_count == 1);
    assert(book->buy_levels[0].price == 100.0);
    assert(book->buy_levels[0].total_quantity == 10);
    assert(book->buy_levels[0].order_count == 1);
    assert(book->order_count == 1);
    
    // Add a sell order
    Order sell_order;
    strcpy(sell_order.id, "S1");
    strcpy(sell_order.symbol, "TEST");
    sell_order.side = SELL;
    sell_order.price = 101.0;
    sell_order.quantity = 5;
    
    add_order(book, &sell_order);
    assert(book->sell_level_count == 1);
    assert(book->sell_levels[0].price == 101.0);
    assert(book->sell_levels[0].total_quantity == 5);
    assert(book->sell_levels[0].order_count == 1);
    assert(book->order_count == 2);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_match_orders() {
    printf("Testing match_orders... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add orders that should match
    Order buy_order;
    strcpy(buy_order.id, "B1");
    strcpy(buy_order.symbol, "TEST");
    buy_order.side = BUY;
    buy_order.price = 101.0;
    buy_order.quantity = 10;
    
    Order sell_order;
    strcpy(sell_order.id, "S1");
    strcpy(sell_order.symbol, "TEST");
    sell_order.side = SELL;
    sell_order.price = 100.0;
    sell_order.quantity = 5;
    
    Order* b_order = add_order(book, &buy_order);
    Order* s_order = add_order(book, &sell_order);
    
    // Check that orders matched
    assert(book->order_count == 2);
    assert(book->buy_levels[0].total_quantity == 5);  // 10 - 5
    assert(book->sell_levels[0].order_count == 0);     // All sold
    
    // Check order statuses
    assert(b_order->filled_quantity == 5); //Need to check this line; it chronically fails. 2 hours dedicated to this bug :/
    assert(s_order->filled_quantity == 5);
    assert(b_order->status == PARTIALLY_FILLED);
    assert(s_order->status == FILLED);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_cancel_order() {
    printf("Testing cancel_order... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add a buy order
    Order buy_order;
    strcpy(buy_order.id, "B1");
    strcpy(buy_order.symbol, "TEST");
    buy_order.side = BUY;
    buy_order.price = 100.0;
    buy_order.quantity = 10;
    
    add_order(book, &buy_order);
    assert(book->buy_level_count == 1);
    
    // Cancel the order
    cancel_order(book, "B1");
    assert(book->buy_level_count == 0);
    
    // Check order status
    Order* b_order = find_order_by_id(book, "B1");
    assert(b_order->status == CANCELLED);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_modify_order() {
    printf("Testing modify_order... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add a buy order
    Order buy_order;
    strcpy(buy_order.id, "B1");
    strcpy(buy_order.symbol, "TEST");
    buy_order.side = BUY;
    buy_order.price = 100.0;
    buy_order.quantity = 10;
    
    add_order(book, &buy_order);
    assert(book->buy_levels[0].total_quantity == 10);
    
    // Modify quantity only
    modify_order(book, "B1", 15, 100.0);
    assert(book->buy_levels[0].total_quantity == 15);
    
    // Modify price
    modify_order(book, "B1", 15, 105.0);
    assert(book->buy_levels[0].price == 105.0);
    assert(book->buy_levels[0].total_quantity == 15);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_fifo_matching() {
    printf("Testing FIFO matching... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add two buy orders at the same price
    Order buy_order1;
    strcpy(buy_order1.id, "B1");
    strcpy(buy_order1.symbol, "TEST");
    buy_order1.side = BUY;
    buy_order1.price = 100.0;
    buy_order1.quantity = 10;
    
    Order buy_order2;
    strcpy(buy_order2.id, "B2");
    strcpy(buy_order2.symbol, "TEST");
    buy_order2.side = BUY;
    buy_order2.price = 100.0;
    buy_order2.quantity = 10;
    
    add_order(book, &buy_order1);
    add_order(book, &buy_order2);
    
    // Add a sell order that matches
    Order sell_order;
    strcpy(sell_order.id, "S1");
    strcpy(sell_order.symbol, "TEST");
    sell_order.side = SELL;
    sell_order.price = 99.0;
    sell_order.quantity = 10;
    
    add_order(book, &sell_order);
    
    // Check that the first buy order was matched (FIFO)
    Order* b1_order = find_order_by_id(book, "B1");
    Order* b2_order = find_order_by_id(book, "B2");
    
    assert(b1_order->filled_quantity == 10);
    assert(b1_order->status == FILLED);
    assert(b2_order->filled_quantity == 0);
    assert(b2_order->status == OPEN);
    
    free_order_book(book);
    printf("PASSED\n");
}

void test_price_time_priority() {
    printf("Testing price-time priority... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add a buy order at a lower price
    Order buy_order1;
    strcpy(buy_order1.id, "B1");
    strcpy(buy_order1.symbol, "TEST");
    buy_order1.side = BUY;
    buy_order1.price = 99.0;
    buy_order1.quantity = 10;
    
    // Add a buy order at a higher price
    Order buy_order2;
    strcpy(buy_order2.id, "B2");
    strcpy(buy_order2.symbol, "TEST");
    buy_order2.side = BUY;
    buy_order2.price = 100.0;
    buy_order2.quantity = 10;
    
    add_order(book, &buy_order1);
    add_order(book, &buy_order2);
    
    // Add a sell order that matches
    Order sell_order;
    strcpy(sell_order.id, "S1");
    strcpy(sell_order.symbol, "TEST");
    sell_order.side = SELL;
    sell_order.price = 99.5;
    sell_order.quantity = 10;
    
    add_order(book, &sell_order);
    
    // Check that the higher price buy order was matched (price priority)
    Order* b1_order = find_order_by_id(book, "B1");
    Order* b2_order = find_order_by_id(book, "B2");
    
    assert(b1_order->filled_quantity == 0);
    assert(b1_order->status == OPEN);
    assert(b2_order->filled_quantity == 10);
    assert(b2_order->status == FILLED);
    
    free_order_book(book);
    printf("PASSED\n");
}

int main() {
    printf("=== ORDER BOOK TESTS ===\n");
    
    test_create_order_book();
    test_add_order();
    test_match_orders();
    test_cancel_order();
    test_modify_order();
    test_fifo_matching();
    test_price_time_priority();
    
    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}