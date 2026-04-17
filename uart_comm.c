#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <poll.h>

#define BAUD_RATE       B115200
#define READ_TIMEOUT_MS 3000
#define RX_BUF_SIZE     256

int  uart_open(const char *device);
int  uart_config(int fd);
int  uart_send(int fd, const char *msg);
int  uart_receive(int fd);
void uart_close(int fd, struct termios *old_cfg);

int main(int argc, char *argv[])
{
    const char *device = (argc > 1) ? argv[1] : "/dev/ttyUSB0";
    printf("[INFO] Opening UART device: %s\n", device);

    int fd = uart_open(device);
    if (fd < 0) return EXIT_FAILURE;

    struct termios old_cfg;
    tcgetattr(fd, &old_cfg);

    if (uart_config(fd) < 0) { uart_close(fd, &old_cfg); return EXIT_FAILURE; }

    const char *test_msg = "HELLO FROM HOST: ACT FRAMEWORK UART TEST\r\n";
    if (uart_send(fd, test_msg) < 0) { uart_close(fd, &old_cfg); return EXIT_FAILURE; }

    uart_receive(fd);
    uart_close(fd, &old_cfg);
    return EXIT_SUCCESS;
}

int uart_open(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] Cannot open %s: %s\n", device, strerror(errno));
        if (errno == EACCES) fprintf(stderr, "        Tip: run with sudo\n");
        if (errno == ENOENT) fprintf(stderr, "        Tip: check device is plugged in\n");
        return -1;
    }
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    printf("[INFO] Device opened (fd=%d)\n", fd);
    return fd;
}

int uart_config(int fd)
{
    struct termios cfg;
    if (tcgetattr(fd, &cfg) < 0) {
        fprintf(stderr, "[ERROR] tcgetattr failed: %s\n", strerror(errno));
        return -1;
    }
    cfg.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    cfg.c_oflag &= ~OPOST;
    cfg.c_cflag &= ~(CSIZE|PARENB|CSTOPB|CRTSCTS);
    cfg.c_cflag |=  CS8 | CREAD | CLOCAL;
    cfg.c_lflag &= ~(ECHO|ECHOE|ECHONL|ICANON|ISIG|IEXTEN);
    cfg.c_cc[VMIN]  = 0;
    cfg.c_cc[VTIME] = 10;
    cfsetispeed(&cfg, BAUD_RATE);
    cfsetospeed(&cfg, BAUD_RATE);
    if (tcsetattr(fd, TCSAFLUSH, &cfg) < 0) {
        fprintf(stderr, "[ERROR] tcsetattr failed: %s\n", strerror(errno));
        return -1;
    }
    printf("[INFO] UART configured: 115200 baud, 8N1\n");
    return 0;
}

int uart_send(int fd, const char *msg)
{
    size_t len = strlen(msg);
    ssize_t written = write(fd, msg, len);
    if (written < 0) { fprintf(stderr, "[ERROR] write() failed: %s\n", strerror(errno)); return -1; }
    tcdrain(fd);
    printf("[TX]   Sent %zd bytes: %s", written, msg);
    return 0;
}

int uart_receive(int fd)
{
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    char buf[RX_BUF_SIZE];
    int total = 0;
    printf("[INFO] Waiting up to %d ms for response...\n", READ_TIMEOUT_MS);
    while (1) {
        int ret = poll(&pfd, 1, READ_TIMEOUT_MS);
        if (ret < 0) { fprintf(stderr, "[ERROR] poll() failed: %s\n", strerror(errno)); return -1; }
        if (ret == 0) { printf("[INFO] Timeout. Total bytes received: %d\n", total); break; }
        if (pfd.revents & POLLIN) {
            ssize_t n = read(fd, buf, sizeof(buf)-1);
            if (n <= 0) break;
            buf[n] = '\0';
            printf("[RX]   %zd bytes: %s", n, buf);
            total += (int)n;
        }
        if (pfd.revents & (POLLERR|POLLHUP|POLLNVAL)) { fprintf(stderr, "[ERROR] Device error.\n"); return -1; }
    }
    return total;
}

void uart_close(int fd, struct termios *old_cfg)
{
    if (old_cfg) tcsetattr(fd, TCSANOW, old_cfg);
    close(fd);
    printf("[INFO] UART closed.\n");
}
