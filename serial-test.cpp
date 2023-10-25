// https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <sys/ioctl.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define _SERIAL_DEV_ "/dev/ttyS0"

int main(void)
{
    int serial_port = open("/dev/ttyS0", O_RDWR);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if (tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    // ioctl(serial_port, TCGETS, &tty);

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)  (0x80000000)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON; // Canonical mode is disabled. Treat all characters equal and do not wait for a  line to complete

    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    /* Input Modes (c_iflag) */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    /* Output Modes (c_oflag)*/
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN]  = 1;

    // cfsetispeed(&tty, B230400);
    // cfsetospeed(&tty, B230400);

    // //  FOR CUSTOM BAUD
    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= CBAUDEX;
    // tty.c_ispeed = 380400; // What a custom baud rate!
    // tty.c_ospeed = 380400;

    // //FOR CUSTOM BAUD
    // ioctl(serial_port, TCSETS, &tty);

    /** For GNO c Library FOR CUSTUM BAUD*/
    cfsetispeed(&tty, 380400);
    cfsetospeed(&tty, 380400);

    tcflush(serial_port, TCIOFLUSH);
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    char serial_in_buffer[10];

    while (true) {
        if (int received = read(serial_port, serial_in_buffer, 1) > 0) {
            for (int i = 0; i < received; i++) {
                printf("0x%02X ", serial_in_buffer[0]);
            }
            printf("\n");
        }
    }

    return 0;
}
