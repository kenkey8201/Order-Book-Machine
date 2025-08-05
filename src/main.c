#include "../include/utils.h"
#include "../src/orderbook.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[]) {
    printf("=== Order Book Matching Engine ===\n");
    //Create the orderbook
    OrderBook* book = create_order_book("AAPL");
    if (book == NULL) {
        fprintf(stderr, "Failed to create order book\n");
        return EXIT_FAILURE;
    }
    // Loading the samples if the user provided any
    if (argc > 1) {
        if (load_orders_from_csv(book, argv[1]) != 0) {
            fprintf(stderr, "Failed to load orders from the file\n");
        } else {
            printf("Loaded orders from %s\n", argv[1]);
            match_orders(book);
            print_order_book(book);
        }
    }
    //Process the user input
    process_user_input(book);
    //If you allocate it, you gotta free it :D
    free_order_book(book);
    return EXIT_SUCCESS;
}