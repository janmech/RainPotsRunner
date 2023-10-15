#ifndef __OOC_LISTENER__
#define __OSC_LISTENER__
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
#ifdef __cplusplus
extern "C"
{
#endif

    void *osc_lisenter_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif // __OSC_LISTENER__