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
#include <deque>
#include "../TheadClass/ThreadClass.h"

#define OSC_MESSAGE_TYPE_NONE 0
#define OSC_MESSAGE_TYPE_CC 1
#define OSC_MESSAGE_TYPE_PRESET_LOAD 2
#define OSC_MESSAGE_TYPE_PRESET_SAVE 3

typedef struct queue_entry_message_t
{
    uint8_t type = OSC_MESSAGE_TYPE_NONE;
    char *buffer = NULL;
    int buffer_size = 0;

    /* data */
} queue_entry_message_t;

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
    void addToMessageQueue(queue_entry_message_t *message);

    void sendMessage(const char *address, const char *format, ...);

    void inc_val()
    {
        this->inc++;
    }

protected:
    int inc = 0;
    int socket_out = -1;
    struct sockaddr_in addr_out;
    std::deque<queue_entry_message_t*> message_queue;

    int openOutSocket();
    void threadLoop();

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscSender *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__