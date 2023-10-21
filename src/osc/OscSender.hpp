#ifndef __OSC_SENDER__
#define __OSC_SENDER__
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#include <string>
#include <iostream>
#include <cstdarg>
#include <deque>
#include "tinyosc/tinyosc.h"
#include "../data/DataHandler.hpp"
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

typedef struct msg_osc_t
{
    int unit = 0;
    int controller = 0;
    std::string path = "";
    std::string format = "";
    std::string val_string = "";
    float val_float = 0.f;
} msg_osc_t;

class OscSender : public ThreadClass
{
public:
    OscSender(DataHandler *data_handler, bool debug = false)
    {
        this->debug = debug;
        this->data_handler = data_handler;
    }

    ~OscSender()
    { /* empty */
    }
    void addRNBOListenter();
    void addToMessageQueue(queue_entry_message_t *message);
    void sendMessage(const char *address, const char *format, ...);

protected:
    bool debug = false;
    DataHandler *data_handler;
    int socket_out = -1;
    struct sockaddr_in addr_out;
    std::deque<queue_entry_message_t *> message_queue;

    int openOutSocket();
    void threadLoop();
    void getOscMessageData(queue_entry_message_t *queue_message, msg_osc_t *osc_message_data);

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscSender *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__