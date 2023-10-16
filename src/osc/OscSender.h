#ifndef __OSC_SENDER__
#define __OSC_SENDER__
#ifdef __cplusplus
extern "C"
{
#endif

#include "tinyosc/tinyosc.h"

    void *osc_sender_runner(void *arg);
#ifdef __cplusplus
}
#endif
#include <string>
#include <iostream>
#include "../TheadClass/ThreadClass.h"

class OscSender : public ThreadClass
{
public:
    OscSender()
    { /* empty */
    }
    ~OscSender()
    { /* empty */
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

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscSender *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__