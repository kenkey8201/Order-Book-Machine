#include "../include/utils.h"
#include "../src/orderbook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize price levels
void initialize_price_levels(PriceLevel* levels, int max_levels) {
    for (int i = 0; i < max_levels; i++) {
        levels[i].price = 0.0;
        levels[i].total_quantity = 0;
        levels[i].orders = NULL;
        levels[i].order_count = 0;
    }
}

// Add an order to a price level
void add_to_price_level(PriceLevel* level, Order* order) {
    level->order_count++;
    level->orders = realloc(level->orders, level->order_count * sizeof(Order));
    if (level->orders == NULL) {
        perror("Failed to allocate memory for orders");
        exit(EXIT_FAILURE);
    }
    
    // Add order to the end (FIFO)
    memcpy(&level->orders[level->order_count - 1], order, sizeof(Order));
    level->total_quantity += order->quantity - order->filled_quantity;
}

// Remove an order from a price level
void remove_from_price_level(PriceLevel* level, const char* order_id) {
    for (int i = 0; i < level->order_count; i++) {
        if (strcmp(level->orders[i].id, order_id) == 0) {
            // Update total quantity
            level->total_quantity -= (level->orders[i].quantity - level->orders[i].filled_quantity);
            
            // Shift remaining orders
            for (int j = i; j < level->order_count - 1; j++) {
                memcpy(&level->orders[j], &level->orders[j + 1], sizeof(Order));
            }
            
            level->order_count--;
            level->orders = realloc(level->orders, level->order_count * sizeof(Order));
            return;
        }
    }
}

// Sort price levels (descending for buy, ascending for sell)
void sort_price_levels(PriceLevel* levels, int count, OrderSide side) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if ((side == BUY && levels[j].price < levels[j + 1].price) ||
                (side == SELL && levels[j].price > levels[j + 1].price)) {
                // Swap price levels
                PriceLevel temp = levels[j];
                levels[j] = levels[j + 1];
                levels[j + 1] = temp;
            }
        }
    }
}

// Find the index of a price level with a specific price
int find_price_level_index(PriceLevel* levels, int count, double price) {
    for (int i = 0; i < count; i++) {
        if (levels[i].price == price) {
            return i;
        }
    }
    return -1;
}

// Execute a trade between a buy and a sell order
void execute_trade(OrderBook* book, Order* buy_order, Order* sell_order, int quantity) {
    printf("TRADE: %s @ %.2f, Qty: %d\n", book->symbol, sell_order->price, quantity);
    
    // Update filled quantities
    buy_order->filled_quantity += quantity;
    sell_order->filled_quantity += quantity;
    
    // Update order statuses
    update_order_status(buy_order);
    update_order_status(sell_order);
}

// Update the status of an order based on filled quantity
void update_order_status(Order* order) {
    if (order->filled_quantity == 0) {
        order->status = OPEN;
    } else if (order->filled_quantity == order->quantity) {
        order->status = FILLED;
    } else {
        order->status = PARTIALLY_FILLED;
    }
}

// Clean up filled orders from the order book
void cleanup_filled_orders(OrderBook* book) {
    // Clean up buy side
    for (int i = 0; i < book->buy_level_count; i++) {
        for (int j = 0; j < book->buy_levels[i].order_count; j++) {
            if (book->buy_levels[i].orders[j].status == FILLED) {
                remove_from_price_level(&book->buy_levels[i], book->buy_levels[i].orders[j].id);
                j--; // Check the same index again after removal
            }
        }
        
        // Remove empty price levels
        if (book->buy_levels[i].order_count == 0) {
            for (int k = i; k < book->buy_level_count - 1; k++) {
                book->buy_levels[k] = book->buy_levels[k + 1];
            }
            book->buy_level_count--;
            i--; // Check the same index again
        }
    }
    
    // Clean up sell side
    for (int i = 0; i < book->sell_level_count; i++) {
        for (int j = 0; j < book->sell_levels[i].order_count; j++) {
            if (book->sell_levels[i].orders[j].status == FILLED) {
                remove_from_price_level(&book->sell_levels[i], book->sell_levels[i].orders[j].id);
                j--; // Check the same index again after removal
            }
        }
        
        // Remove empty price levels
        if (book->sell_levels[i].order_count == 0) {
            for (int k = i; k < book->sell_level_count - 1; k++) {
                book->sell_levels[k] = book->sell_levels[k + 1];
            }
            book->sell_level_count--;
            i--; // Check the same index again
        }
    }
}