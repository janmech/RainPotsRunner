#ifndef __OSC_LISTENER__
#define __OSC_LISTENER__
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "tinyosc/tinyosc.h"

    void *osc_lisenter_run(void *arg);

#ifdef __cplusplus
}
#endif

#endif // __OSC_LISTENER__