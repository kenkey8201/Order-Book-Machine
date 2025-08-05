#include "../include/utils.h"
#include "../src/orderbook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Create a new order book
OrderBook* create_order_book(const char* symbol) {
    OrderBook* book = malloc(sizeof(OrderBook));
    if (book == NULL) {
        perror("Failed to allocate memory for order book");
        return NULL;
    }
    
    strncpy(book->symbol, symbol, MAX_SYMBOL_LENGTH - 1);
    book->symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
    
    // Allocate memory for price levels
    book->buy_levels = malloc(MAX_PRICE_LEVELS * sizeof(PriceLevel));
    book->sell_levels = malloc(MAX_PRICE_LEVELS * sizeof(PriceLevel));
    
    if (book->buy_levels == NULL || book->sell_levels == NULL) {
        perror("Failed to allocate memory for price levels");
        free(book->buy_levels);
        free(book->sell_levels);
        free(book);
        return NULL;
    }
    
    // Initialize price levels
    initialize_price_levels(book->buy_levels, MAX_PRICE_LEVELS);
    initialize_price_levels(book->sell_levels, MAX_PRICE_LEVELS);
    book->buy_level_count = 0;
    book->sell_level_count = 0;
    
    // Allocate memory for all orders
    book->all_orders = malloc(MAX_ORDERS * sizeof(Order));
    if (book->all_orders == NULL) {
        perror("Failed to allocate memory for orders");
        free(book->buy_levels);
        free(book->sell_levels);
        free(book);
        return NULL;
    }
    book->order_count = 0;
    
    return book;
}

// Free order book memory
void free_order_book(OrderBook* book) {
    if (book != NULL) {
        // Free buy levels
        for (int i = 0; i < book->buy_level_count; i++) {
            free(book->buy_levels[i].orders);
        }
        free(book->buy_levels);
        
        // Free sell levels
        for (int i = 0; i < book->sell_level_count; i++) {
            free(book->sell_levels[i].orders);
        }
        free(book->sell_levels);
        
        // Free all orders
        free(book->all_orders);
        
        // Free the book itself
        free(book);
    }
}

// Add an order to the order book
Order* add_order(OrderBook* book, Order* order) {
    if (book->order_count >= MAX_ORDERS) {
        fprintf(stderr, "Order book is full\n");
        return NULL;
    }
    
    // Set timestamp
    order->timestamp = time(NULL);
    order->status = OPEN;
    order->filled_quantity = 0;
    
    // Add to all orders
    memcpy(&book->all_orders[book->order_count], order, sizeof(Order));
    book->order_count++;
    
    // Get a pointer to the order in the book
    Order* book_order = &book->all_orders[book->order_count - 1];
    
    // Determine which side to add to
    PriceLevel* levels;
    int* level_count;
    
    if (book_order->side == BUY) {
        levels = book->buy_levels;
        level_count = &book->buy_level_count;
    } else {
        levels = book->sell_levels;
        level_count = &book->sell_level_count;
    }
    
    // Check if price level already exists
    int level_index = find_price_level_index(levels, *level_count, book_order->price);
    
    if (level_index == -1) {
        // Create new price level
        if (*level_count >= MAX_PRICE_LEVELS) {
            fprintf(stderr, "Maximum price levels reached\n");
            return NULL;
        }
        
        level_index = *level_count;
        levels[level_index].price = book_order->price;
        levels[level_index].total_quantity = 0;
        levels[level_index].orders = NULL;
        levels[level_index].order_count = 0;
        (*level_count)++;
    }
    
    // Add order to price level
    add_to_price_level(&levels[level_index], book_order);
    
    // Sort price levels
    sort_price_levels(levels, *level_count, book_order->side);
    
    // Try to match orders
    match_orders(book);
    
    return book_order;
}

// Cancel an order
void cancel_order(OrderBook* book, const char* order_id) {
    Order* order = find_order_by_id(book, order_id);
    if (order == NULL) {
        printf("Order not found: %s\n", order_id);
        return;
    }
    
    if (order->status == FILLED) {
        printf("Cannot cancel filled order: %s\n", order_id);
        return;
    }
    
    order->status = CANCELLED;
    
    // Remove from price levels
    PriceLevel* levels;
    int* level_count;
    
    if (order->side == BUY) {
        levels = book->buy_levels;
        level_count = &book->buy_level_count;
    } else {
        levels = book->sell_levels;
        level_count = &book->sell_level_count;
    }
    
    int level_index = find_price_level_index(levels, *level_count, order->price);
    if (level_index != -1) {
        remove_from_price_level(&levels[level_index], order_id);
        
        // Remove empty price levels
        if (levels[level_index].order_count == 0) {
            for (int i = level_index; i < *level_count - 1; i++) {
                levels[i] = levels[i + 1];
            }
            (*level_count)--;
        }
    }
    
    printf("Cancelled order: %s\n", order_id);
}

// Modify an order
void modify_order(OrderBook* book, const char* order_id, int new_quantity, double new_price) {
    Order* order = find_order_by_id(book, order_id);
    if (order == NULL) {
        printf("Order not found: %s\n", order_id);
        return;
    }
    
    if (order->status == FILLED) {
        printf("Cannot modify filled order: %s\n", order_id);
        return;
    }
    
    // If price is changing, we need to remove and re-add
    if (order->price != new_price) {
        // Cancel the original order
        Order temp_order;
        memcpy(&temp_order, order, sizeof(Order));
        cancel_order(book, order_id);
        
        // Create a new order with the updated price and quantity
        temp_order.price = new_price;
        temp_order.quantity = new_quantity;
        add_order(book, &temp_order);
    } else if (order->quantity != new_quantity) {
        // Only quantity is changing, update it directly
        int quantity_diff = new_quantity - order->quantity;
        order->quantity = new_quantity;
        
        // Update the price level total quantity
        PriceLevel* levels;
        int* level_count;
        
        if (order->side == BUY) {
            levels = book->buy_levels;
            level_count = &book->buy_level_count;
        } else {
            levels = book->sell_levels;
            level_count = &book->sell_level_count;
        }
        
        int level_index = find_price_level_index(levels, *level_count, order->price);
        if (level_index != -1) {
            levels[level_index].total_quantity += quantity_diff;
        }
        
        printf("Modified order quantity: %s, New Qty: %d\n", order_id, new_quantity);
    }
}

// Match orders in the order book
void match_orders(OrderBook* book) {
    // Match while we have both buy and sell levels
    while (book->buy_level_count > 0 && book->sell_level_count > 0) {
        PriceLevel* best_buy = &book->buy_levels[0];  // Highest buy price
        PriceLevel* best_sell = &book->sell_levels[0];  // Lowest sell price
        
        // Check if we can match
        if (best_buy->price >= best_sell->price) {
            // Get the first order in each price level (FIFO)
            Order* buy_order = &best_buy->orders[0];
            Order* sell_order = &best_sell->orders[0];
            
            // Calculate trade quantity
            int buy_qty = buy_order->quantity - buy_order->filled_quantity;
            int sell_qty = sell_order->quantity - sell_order->filled_quantity;
            int trade_qty = (buy_qty < sell_qty) ? buy_qty : sell_qty;
            
            // Execute the trade
            execute_trade(book, buy_order, sell_order, trade_qty);
            
            // Update price level quantities
            best_buy->total_quantity -= trade_qty;
            best_sell->total_quantity -= trade_qty;
            
            // Clean up filled orders
            cleanup_filled_orders(book);
        } else {
            // No more matches possible
            break;
        }
    }
}

// Print the order book (L2 view)
void print_order_book(const OrderBook* book) {
    printf("\n=== ORDER BOOK: %s ===\n", book->symbol);
    printf("%-10s %-10s %-10s\n", "Price", "Quantity", "Count");
    
    // Print sell levels (from high to low)
    for (int i = book->sell_level_count - 1; i >= 0; i--) {
        printf("%-10.2f %-10d %-10d\n", 
               book->sell_levels[i].price, 
               book->sell_levels[i].total_quantity, 
               book->sell_levels[i].order_count);
    }
    
    printf("--------------------\n");
    
    // Print buy levels (from high to low)
    for (int i = 0; i < book->buy_level_count; i++) {
        printf("%-10.2f %-10d %-10d\n", 
               book->buy_levels[i].price, 
               book->buy_levels[i].total_quantity, 
               book->buy_levels[i].order_count);
    }
    
    printf("========================\n");
}

// Print an order
void print_order(const Order* order) {
    const char* side_str = (order->side == BUY) ? "BUY" : "SELL";
    const char* status_str;
    
    switch (order->status) {
        case OPEN: status_str = "OPEN"; break;
        case FILLED: status_str = "FILLED"; break;
        case PARTIALLY_FILLED: status_str = "PARTIALLY FILLED"; break;
        case CANCELLED: status_str = "CANCELLED"; break;
        default: status_str = "UNKNOWN";
    }
    
    printf("Order ID: %s, Symbol: %s, Side: %s, Price: %.2f, Quantity: %d, Filled: %d, Status: %s\n",
           order->id, order->symbol, side_str, order->price, order->quantity, order->filled_quantity, status_str);
}

// Find an order by ID
Order* find_order_by_id(OrderBook* book, const char* order_id) {
    for (int i = 0; i < book->order_count; i++) {
        if (strcmp(book->all_orders[i].id, order_id) == 0) {
            return &book->all_orders[i];
        }
    }
    return NULL;
}

// Load orders from a CSV file
int load_orders_from_csv(OrderBook* book, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    
    char line[256];
    int line_num = 0;
    
    // Skip header line
    fgets(line, sizeof(line), file);
    line_num++;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Parse CSV line
        char id[MAX_ID_LENGTH];
        char symbol[MAX_SYMBOL_LENGTH];
        char side_str[8];
        double price;
        int quantity;
        
        if (sscanf(line, "%[^,],%[^,],%[^,],%lf,%d", id, symbol, side_str, &price, &quantity) != 5) {
            fprintf(stderr, "Invalid format at line %d: %s", line_num, line);
            continue;
        }
        
        // Create order
        Order order;
        strncpy(order.id, id, MAX_ID_LENGTH - 1);
        order.id[MAX_ID_LENGTH - 1] = '\0';
        strncpy(order.symbol, symbol, MAX_SYMBOL_LENGTH - 1);
        order.symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
        
        // Parse side
        if (strcasecmp(side_str, "BUY") == 0) {
            order.side = BUY;
        } else if (strcasecmp(side_str, "SELL") == 0) {
            order.side = SELL;
        } else {
            fprintf(stderr, "Invalid side at line %d: %s\n", line_num, side_str);
            continue;
        }
        
        order.price = price;
        order.quantity = quantity;
        order.filled_quantity = 0;
        order.timestamp = time(NULL);
        order.status = OPEN;
        
        // Add to order book
        add_order(book, &order);
    }
    
    fclose(file);
    return 0;
}

// Save orders to a CSV file
int save_orders_to_csv(const OrderBook* book, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }
    
    // Write header
    fprintf(file, "ID,Symbol,Side,Price,Quantity,Filled,Status\n");
    
    // Write orders
    for (int i = 0; i < book->order_count; i++) {
        const Order* order = &book->all_orders[i];
        const char* side_str = (order->side == BUY) ? "BUY" : "SELL";
        const char* status_str;
        
        switch (order->status) {
            case OPEN: status_str = "OPEN"; break;
            case FILLED: status_str = "FILLED"; break;
            case PARTIALLY_FILLED: status_str = "PARTIALLY FILLED"; break;
            case CANCELLED: status_str = "CANCELLED"; break;
            default: status_str = "UNKNOWN";
        }
        
        fprintf(file, "%s,%s,%s,%.2f,%d,%d,%s\n",
                order->id, order->symbol, side_str, order->price,
                order->quantity, order->filled_quantity, status_str);
    }
    
    fclose(file);
    return 0;
}

// Process user input
void process_user_input(OrderBook* book) {
    char input[256];
    
    while (1) {
        printf("\nEnter command (help for list of commands): ");
        fgets(input, sizeof(input), stdin);
        
        // Remove newline character
        input[strcspn(input, "\n")] = '\0';
        
        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }
        
        // Parse command
        char command[32];
        sscanf(input, "%31s", command);
        
        if (strcasecmp(command, "help") == 0) {
            display_help();
        } else if (strcasecmp(command, "buy") == 0) {
            char id[MAX_ID_LENGTH];
            double price;
            int quantity;
            
            if (sscanf(input, "%*s %15s %lf %d", id, &price, &quantity) != 3) {
                printf("Invalid format. Usage: buy <id> <price> <quantity>\n");
                continue;
            }
            
            Order order;
            strncpy(order.id, id, MAX_ID_LENGTH - 1);
            order.id[MAX_ID_LENGTH - 1] = '\0';
            strncpy(order.symbol, book->symbol, MAX_SYMBOL_LENGTH - 1);
            order.symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
            order.side = BUY;
            order.price = price;
            order.quantity = quantity;
            
            add_order(book, &order);
            print_order_book(book);
        } else if (strcasecmp(command, "sell") == 0) {
            char id[MAX_ID_LENGTH];
            double price;
            int quantity;
            
            if (sscanf(input, "%*s %15s %lf %d", id, &price, &quantity) != 3) {
                printf("Invalid format. Usage: sell <id> <price> <quantity>\n");
                continue;
            }
            
            Order order;
            strncpy(order.id, id, MAX_ID_LENGTH - 1);
            order.id[MAX_ID_LENGTH - 1] = '\0';
            strncpy(order.symbol, book->symbol, MAX_SYMBOL_LENGTH - 1);
            order.symbol[MAX_SYMBOL_LENGTH - 1] = '\0';
            order.side = SELL;
            order.price = price;
            order.quantity = quantity;
            
            add_order(book, &order);
            print_order_book(book);
        } else if (strcasecmp(command, "cancel") == 0) {
            char id[MAX_ID_LENGTH];
            
            if (sscanf(input, "%*s %15s", id) != 1) {
                printf("Invalid format. Usage: cancel <id>\n");
                continue;
            }
            
            cancel_order(book, id);
            print_order_book(book);
        } else if (strcasecmp(command, "modify") == 0) {
            char id[MAX_ID_LENGTH];
            int quantity;
            double price;
            
            if (sscanf(input, "%*s %15s %d %lf", id, &quantity, &price) != 3) {
                printf("Invalid format. Usage: modify <id> <new_quantity> <new_price>\n");
                continue;
            }
            
            modify_order(book, id, quantity, price);
            print_order_book(book);
        } else if (strcasecmp(command, "book") == 0) {
            print_order_book(book);
        } else if (strcasecmp(command, "order") == 0) {
            char id[MAX_ID_LENGTH];
            
            if (sscanf(input, "%*s %15s", id) != 1) {
                printf("Invalid format. Usage: order <id>\n");
                continue;
            }
            
            Order* order = find_order_by_id(book, id);
            if (order != NULL) {
                print_order(order);
            } else {
                printf("Order not found: %s\n", id);
            }
        } else if (strcasecmp(command, "save") == 0) {
            char filename[256];
            
            if (sscanf(input, "%*s %255s", filename) != 1) {
                printf("Invalid format. Usage: save <filename>\n");
                continue;
            }
            
            if (save_orders_to_csv(book, filename) == 0) {
                printf("Orders saved to %s\n", filename);
            }
        } else if (strcasecmp(command, "load") == 0) {
            char filename[256];
            
            if (sscanf(input, "%*s %255s", filename) != 1) {
                printf("Invalid format. Usage: load <filename>\n");
                continue;
            }
            
            if (load_orders_from_csv(book, filename) == 0) {
                printf("Orders loaded from %s\n", filename);
                print_order_book(book);
            }
        } else if (strcasecmp(command, "exit") == 0 || strcasecmp(command, "quit") == 0) {
            break;
        } else {
            printf("Unknown command: %s. Type 'help' for a list of commands.\n", command);
        }
    }
}

// Display help information
void display_help() {
    printf("\n=== ORDER BOOK COMMANDS ===\n");
    printf("buy <id> <price> <quantity>   - Add a buy order\n");
    printf("sell <id> <price> <quantity>  - Add a sell order\n");
    printf("cancel <id>                  - Cancel an order\n");
    printf("modify <id> <qty> <price>    - Modify an order\n");
    printf("book                         - Display the order book\n");
    printf("order <id>                   - Display order details\n");
    printf("save <filename>              - Save orders to CSV file\n");
    printf("load <filename>              - Load orders from CSV file\n");
    printf("help                         - Show this help message\n");
    printf("exit/quit                    - Exit the program\n");
    printf("=============================\n");
}