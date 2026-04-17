# RISC-V ACT Framework — UART Interface

A C program that initializes and configures a UART interface on Linux using the `termios` API.

## Features
- Configures UART: 115200 baud, 8N1, no flow control
- Transmits a test message over UART
- Receives response using non-blocking `poll()`
- Graceful error handling

## Build
- gcc -o uart_comm uart_comm.c

## Run
- sudo ./uart_comm /dev/ttyUSB0