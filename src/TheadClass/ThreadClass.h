#ifndef __THREAD_CLASS__
#define __THREAD_CLASS__
#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif

#include <iostream>
#include <string>

#define THREAD_LOOP_SLEEP_US 1000

class ThreadClass {
public:
    ThreadClass()
    { /* empty */
    }
    virtual ~ThreadClass()
    { /* empty */
    }

    /** Returns true if the thread was successfully started, false if there was an error starting the thread */
    bool start()
    {
        return (pthread_create(&_thread, NULL, InternalThreadFunc, this) == 0);
    }

    void stop()
    {
        keep_running = false;
        (void)pthread_join(_thread, NULL);
    }

    /** Will not return until the internal thread has exited. */
    void WaitForInternalThreadToExit()
    {
        (void)pthread_join(_thread, NULL);
    }

protected:
    virtual void threadLoop() = 0;
    bool         keep_running = true;

private:
    static void* InternalThreadFunc(void* This)
    {
        ((ThreadClass*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __THREAD_CLASS__