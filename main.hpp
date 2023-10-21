#ifndef __MAIN_PROGRAMM__
#define __MAIN_PROGRAMM__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
// #include <fcntl.h>
// #include <termios.h>
#include "src/test.h"

#ifdef __cplusplus
}
#include <iostream>
#include "src/osc/OscSender.hpp"
#include "src/osc/OscListener.hpp"
#include "src/data/DataHandler.hpp"
#include "src/serial/SerialConnector.hpp"
#endif


#endif //__MAIN_PROGRAMM__