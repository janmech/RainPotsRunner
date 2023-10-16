#ifndef __OSC_SENDER__
#define __OSC_SENDER__
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
#include <pthread.h>
#include "tinyosc/tinyosc.h"

    void *osc_sender_runner(void *arg);
#ifdef __cplusplus
}
#endif
#include <string>
#include <iostream>

class OscSender
{
public:
    OscSender()
    { /* empty */
    }
    ~OscSender()
    { /* empty */
    }
    
    /** Returns true if the thread was successfully started, false if there was an error starting the thread */
    bool start()
    {
        return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
    }

    void stop()
    {
        this->keep_running = false;
        (void)pthread_join(_thread, NULL);
    }

    /** Will not return until the internal thread has exited. */
    void WaitForInternalThreadToExit()
    {
        (void)pthread_join(_thread, NULL);
    }

    void inc_val()
    {
        this->inc++;
    }

protected:
    void threadLoop()
    {
        while (this->keep_running)
        {
            printf("OscSender::run....%d\n", this->inc);
            sleep(1);
        }
        std::cout << "\tOscSender Terminated" << std::endl;
    }

    int inc = 0;
    bool keep_running = true;

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscSender *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__