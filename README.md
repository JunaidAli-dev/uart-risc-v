# RISC-V ACT Framework — UART Interface

A robust C utility designed to initialize, configure, and validate UART communication on Linux using the POSIX `termios` API. This tool serves as a host-side validation script for the RISC-V Architecture Test (ACT) Framework, ensuring reliable serial communication with RISC-V targets.

## Features
* **Hardware Configuration:** Configures UART to standard embedded specifications (115200 baud, 8 data bits, No parity, 1 stop bit, no hardware flow control).
* **Non-Blocking I/O:** Utilizes `poll()` with a defined timeout for robust, non-blocking asynchronous reception.
* **State Preservation:** Automatically saves and restores the host machine's original terminal attributes upon exit or error.
* **Graceful Error Handling:** Provides descriptive `strerror` outputs for debugging permission or connection issues.

## Prerequisites
* GCC or Clang compiler
* Linux environment (or GitHub Codespaces)

## Build Instructions
Compile the source code using `gcc`:

```bash
gcc -Wall -Wextra -O2 -o uart_comm uart_comm.c
```

## Running the Application

### Option 1: Physical Hardware (RISC-V Board)
Connect your RISC-V development board (e.g., via a USB-to-UART bridge) and run the executable with elevated privileges to access the device file:

```bash
sudo ./uart_comm /dev/ttyUSB0
```

(Note: Replace `/dev/ttyUSB0` with your specific device node if different, such as `/dev/ttyACM0`)

### Option 2: Virtual Simulation Testing (Local/Codespace)
If you do not have physical hardware attached, you can simulate a UART loopback connection using `socat` to verify the logic.

1. Open a new terminal and create a linked pair of virtual serial ports:

```bash
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

   *Note the output. It will generate two connected endpoints, e.g., `/dev/pts/1` and `/dev/pts/2`.*

2. Open a second terminal to act as the "RISC-V Target" and listen on the second port:

```bash
cat < /dev/pts/2
```

3. Run the compiled UART script on the first port:

```bash
./uart_comm /dev/pts/1
```
