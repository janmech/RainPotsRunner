/*************************
 * UART For Raspberry Pi (2014/10/10)
 * gcc version 4.6.3 (Debian 4.6.3-14+rpi1)
 *
 * copyright ultra_mcu@msn.com
 * www.appstack.in.th
 * https://github.com/ultramcu/AppStackUART
 * ***********************/

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "src/serial/app_stack_uart.h"

#define _SERIAL_DEV_ "/dev/ttyS0"

st_uart MY_UART;

int32_t my_uart_callback_function(void *arg)
{
    st_uart_data *data = (st_uart_data *)arg;
    printf("Received %2d Bye, Data : %s\r\n", data->count, data->data);
}

int main(void)
{
    // system("clear");
    printf("********************************\r\n");
    printf("*   Raspberry Pi UART library  *\r\n");
    printf("*                              *\r\n");
    printf("*      www.appstack.in.th      *\r\n");
    printf("*         (2014/10/10)         *\r\n");
    printf("********************************\r\n");
    printf("\r\n");

    if (uart_begin(&MY_UART, _SERIAL_DEV_, 11520, &my_uart_callback_function) != _TRUE_)
    {
        printf("Error : uart_begin failed\r\n");
        return 0;
    }

    while (1)
    {
        printf("sending: A\n");
        serial_send(&MY_UART, "A");
        sleep(5);
    }

    return 0;
}
