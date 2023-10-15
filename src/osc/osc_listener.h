#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "tinyosc/tinyosc.h"
#ifndef __UOC_LISTENER__
#define __OSC_LISTENER__

void *osc_lisenter_thread(void *arg);

#endif