# Order Book Matching Engine ⚙️

A basic simulation of a stock exchange order book implemented in C.

## Features
- Handles limit buy/sell orders
- FIFO queueing for matching
- Price-time priority matching
- CLI interface for input and matching status
- Simulates L2 book view
- Order modification and cancellation
- Load/save orders from/to CSV files

## Why
Simulates how real exchanges work under the hood. Useful for low-latency finance roles.

## Prerequisites

This project requires a C compiler and make build system. The recommended setup is:

- **Windows**: Install [MSYS2](https://www.msys2.org/) with the MinGW-w64 toolchain
  ```bash
  pacman -S mingw-w64-x86_64-toolchain
  pacman -S make
  ```

- **Linux**: Install build essentials
  ```bash
  # Ubuntu/Debian
  sudo apt-get install build-essential
  
  # Fedora/CentOS
  sudo dnf groupinstall "Development Tools"
  ```

- **macOS**: Install Xcode Command Line Tools
  ```bash
  xcode-select --install
  ```

## Compile

### Using Make (Recommended)

```bash
# Clone the repository
git clone https://github.com/yourusername/orderbook-matching-c.git
cd orderbook-matching-c

# Build the project
make

# Run the application
./orderbook

# Run with sample data
./orderbook data/sample_orders.csv
```

### Alternative Compilation (Without Make)

If you don't have make installed, you can compile directly:

```bash
# Compile the main application
gcc -Wall -Wextra -std=c99 -I./include src/main.c src/orderbook.c src/utils.c -o orderbook

# Run the application
./orderbook data/sample_orders.csv
```

## Testing

To run the test suite:

```bash
# Build and run tests
make test
./orderbook_test
```

## Usage

### Commands

- `buy <id> <price> <quantity>` - Add a buy order
- `sell <id> <price> <quantity>` - Add a sell order
- `cancel <id>` - Cancel an order
- `modify <id> <qty> <price>` - Modify an order
- `book` - Display the order book
- `order <id>` - Display order details
- `save <filename>` - Save orders to CSV file
- `load <filename>` - Load orders from CSV file
- `help` - Show this help message
- `exit/quit` - Exit the program

### Example Session

```
=== ORDER BOOK MATCHING ENGINE ===

Enter command (help for list of commands): help

=== ORDER BOOK COMMANDS ===
buy <id> <price> <quantity>   - Add a buy order
sell <id> <price> <quantity>  - Add a sell order
cancel <id>                  - Cancel an order
modify <id> <qty> <price>    - Modify an order
book                         - Display the order book
order <id>                   - Display order details
save <filename>              - Save orders to CSV file
load <filename>              - Load orders from CSV file
help                         - Show this help message
exit/quit                    - Exit the program
=============================

Enter command (help for list of commands): buy B1 150.25 100
TRADE: AAPL @ 150.25, Qty: 50

=== ORDER BOOK: AAPL ===
Price     Quantity  Count    
150.50    50        1        
------------------------------------
150.25    50        1        
========================

Enter command (help for list of commands): exit
```

## Architecture

The order book matching engine consists of the following components:

1. **Order**: Represents a single order with attributes like ID, symbol, side (buy/sell), price, quantity, etc.
2. **PriceLevel**: Groups orders at the same price level, maintaining total quantity and order count.
3. **OrderBook**: Maintains buy and sell price levels, all orders, and provides matching functionality.

### Matching Algorithm

The engine uses a price-time priority algorithm:
1. Best prices are matched first (highest bid, lowest ask)
2. At the same price level, orders are matched in FIFO order
3. Partial fills are supported
4. Orders can be modified or cancelled

## File Structure

```
orderbook-matching-c/
├── src/
│   ├── orderbook.c     # Core order book functionality
│   ├── orderbook.h     # Header for order book functions
│   ├── main.c          # Main program entry point
│   └── utils.c         # Utility functions and CLI interface
├── include/
│   └── utils.h         # Header for utility functions and data structures
├── test/
│   └── orderbook_test.c # Unit tests
├── data/
│   └── sample_orders.csv # Sample order data
├── README.md
├── Makefile
└── .gitignore
```

## Troubleshooting

### Makefile Issues

If you encounter "missing separator" errors with the Makefile:
- Ensure you're using a proper Unix-like environment (MSYS2 on Windows)
- The Makefile requires tabs, not spaces for indentation
- If issues persist, use the direct compilation method shown above

### Windows-Specific Issues

On Windows, ensure you're using the **MSYS2 MinGW 64-bit** terminal, not:
- Regular Windows Command Prompt
- PowerShell
- Git Bash (unless you've installed MinGW-w64 there)

### Compilation Errors

If you encounter compilation errors:
1. Verify all prerequisites are installed
2. Ensure you're in the correct directory
3. Check that all source files exist in the expected locations

## Future Enhancements

1. Market order support
2. Order types (stop-loss, stop-limit, etc.)
3. Multi-symbol support
4. Performance optimizations
5. Networking for remote access
6. Persistence to database
7. Real-time data feed integration

## Contributing

Feel free to submit issues and enhancement requests!
