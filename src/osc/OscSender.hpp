#ifndef __OSC_SENDER__
#define __OSC_SENDER__
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#include "tinyosc/tinyosc.h"
#include <string>
#include <iostream>
#include <cstdarg>
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
    void addRNBOListenter();

    void sendMessage(const char *address, const char *format, ...);

    void inc_val()
    {
        this->inc++;
    }

protected:
    int inc = 0;
    int socket_out = -1;
    struct sockaddr_in addr_out;

    int openOutSocket();
    void threadLoop()
    {
        while (this->keep_running)
        {
            // printf("OscSender::run....%d\n", this->inc);
            sleep(1);
        }
        close(this->socket_out);
        std::cout << "\tOscSender Terminated" << std::endl;
    }

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscSender *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__