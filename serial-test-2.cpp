// C library headers
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Linux headers
#include <asm/termios.h> // Contains POSIX terminal control definitions
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <unistd.h> // write(), read(), close()

namespace foo {
#include <sys/ioctl.h>
}

int main()
{
    printf("Running test....\n");
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyS0", O_RDWR);
    if (serial_port < 0) {
        printf("Error opening Serial Port");
        return 1;
    }

    // Create new termios struct, we call it 'tty' for convention
    struct termios2 tty;

    // Read in existing settings, and handle any error
    foo::ioctl(serial_port, TCGETS2, &tty);

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN]  = 0;

    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= CBAUDEX;
    tty.c_ispeed = 380400; // What a custom baud rate!
    tty.c_ospeed = 380400;

    // tty.c_ispeed = 230400; // What a custom baud rate!
    // tty.c_ospeed = 230400;

    foo::ioctl(serial_port, TCSETS2, &tty);

    char serial_in_buffer[10];
    int  received;

    while (true) {

        if (received = read(serial_port, serial_in_buffer, 1) > 0) {
            for (int i = 0; i < received; i++) {
                printf("0x%02X ", serial_in_buffer[0]);
            }
            printf("\n");
        }
    }

    close(serial_port);
    return 0; // success
};