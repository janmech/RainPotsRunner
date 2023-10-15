#ifndef __MAIN_PROGRAMM__
#define __MAIN_PROGRAMM__
#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef __cplusplus
}
#include "src/bash_colors.hpp"
#include "src/data/DataHandler.hpp"
#include "src/osc/OscListener.hpp"
#include "src/osc/OscSender.hpp"
#include "src/serial/SerialConnector.hpp"
#include <iostream>
#endif

#endif //__MAIN_PROGRAMM__