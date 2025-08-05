#include "../include/utils.h"
#include "../src/orderbook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void test_match_orders_debug() {
    printf("Testing match_orders (debug)... ");
    
    OrderBook* book = create_order_book("TEST");
    
    // Add orders that should match
    Order buy_order;
    strcpy(buy_order.id, "B1");
    strcpy(buy_order.symbol, "TEST");
    buy_order.side = BUY;
    buy_order.price = 101.0;
    buy_order.quantity = 10;
    buy_order.filled_quantity = 0;
    
    Order sell_order;
    strcpy(sell_order.id, "S1");
    strcpy(sell_order.symbol, "TEST");
    sell_order.side = SELL;
    sell_order.price = 100.0;
    sell_order.quantity = 5;
    sell_order.filled_quantity = 0;
    
    printf("\nBefore adding orders:\n");
    printf("Buy order: quantity=%d, filled=%d\n", buy_order.quantity, buy_order.filled_quantity);
    printf("Sell order: quantity=%d, filled=%d\n", sell_order.quantity, sell_order.filled_quantity);
    
    add_order(book, &buy_order);
    add_order(book, &sell_order);
    
    printf("\nAfter adding orders:\n");
    printf("Buy order: quantity=%d, filled=%d\n", buy_order.quantity, buy_order.filled_quantity);
    printf("Sell order: quantity=%d, filled=%d\n", sell_order.quantity, sell_order.filled_quantity);
    
    // Check order statuses
    Order* b_order = find_order_by_id(book, "B1");
    Order* s_order = find_order_by_id(book, "S1");
    
    printf("\nAfter matching:\n");
    printf("Buy order: quantity=%d, filled=%d\n", b_order->quantity, b_order->filled_quantity);
    printf("Sell order: quantity=%d, filled=%d\n", s_order->quantity, s_order->filled_quantity);
    
    printf("\nBook state:\n");
    printf("buy_level_count=%d, sell_level_count=%d\n", book->buy_level_count, book->sell_level_count);
    if (book->buy_level_count > 0) {
        printf("Buy level 0: price=%.2f, total_quantity=%d, order_count=%d\n", 
               book->buy_levels[0].price, book->buy_levels[0].total_quantity, book->buy_levels[0].order_count);
    }
    if (book->sell_level_count > 0) {
        printf("Sell level 0: price=%.2f, total_quantity=%d, order_count=%d\n", 
               book->sell_levels[0].price, book->sell_levels[0].total_quantity, book->sell_levels[0].order_count);
    }
    
    free_order_book(book);
    printf("DEBUG test completed\n");
}

int main() {
    printf("=== ORDER BOOK DEBUG TESTS ===\n");
    
    test_match_orders_debug();
    
    printf("=== DEBUG TESTS COMPLETED ===\n");
    return 0;
}
