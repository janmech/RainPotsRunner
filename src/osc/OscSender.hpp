#ifndef __OSC_SENDER__
#define __OSC_SENDER__
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "../TheadClass/ThreadClass.h"
#include "../bash_colors.hpp"
#include "../data/DataHandler.hpp"
#include "../data/TSQueue.hpp"
#include "../rainpot_types.hpp"
#include "tinyosc/tinyosc.h"
#include <cstdarg>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class OscSender : public ThreadClass {
public:
    OscSender(DataHandler* data_handler, bool debug = false)
    {
        this->debug        = debug;
        this->data_handler = data_handler;
    }

    ~OscSender()
    { /* empty */
    }
    void      addRNBOListenter();
    void      addToMessageQueue(queue_entry_message_t* message);
    void      sendMessage(const char* address, const char* format, ...);
    pthread_t getThreadId() { return pthread_self(); }

protected:
    bool                               debug = false;
    DataHandler*                       data_handler;
    int                                socket_out = -1;
    struct sockaddr_in                 addr_out;
    std::deque<queue_entry_message_t*> message_queue;
    TSQueue<queue_entry_message_t*>    ts_message_queue;

    float makeValueFLoat(int unit, int controler, int raw_value);
    int   openOutSocket();
    void  threadLoop();
    void  getOscMessageData(queue_entry_message_t* queue_message, msg_osc_t* osc_message_data);

private:
    static void* InternalThreadEntryFunc(void* This)
    {
        ((OscSender*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_SENDER__
