#ifndef __THREAD_CLASS__
#define __THREAD_CLASS__
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
#ifdef __cplusplus
}
#endif

#include <string>
#include <iostream>

class ThreadClass
{
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
    bool keep_running = true;

private:
    static void *InternalThreadFunc(void *This)
    {
        ((ThreadClass *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __THREAD_CLASS__