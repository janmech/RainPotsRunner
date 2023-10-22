#ifndef __SERIAL_CONNECTOR__
#define __SERIAL_CONNECTOR__
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#include "../TheadClass/ThreadClass.h"
#include "../bash_colors.hpp"
#include "../data/TSQueue.hpp"
#include "../osc/OscSender.hpp"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string_view>
#include <termios.h>

#define SERIAL_PORT_PATH     "/dev/ttyS0"
#define SERIAL_IN_BUFFER_LEN 100

#define MSG_BUFFER_SIZE         100
#define MSG_CC_STATUS_BIT_MASK  0x0B
#define MSG_CC_PACKET_SIZE      4
#define MSG_PGM_STATUS_BIT_MASK 0x0C
#define MSG_PGM_PACKET_SIZE     2
#define MSG_SAVE                0xF4
#define MSG_SAVE_PACKET_SIZE    2

typedef struct serial_queue_entry_t {
    char* buffer      = NULL;
    int   buffer_size = 0;

} serial_queue_entry_t;

class SerialConnector : public ThreadClass {
public:
    SerialConnector(OscSender* osc_sender, bool debug = false)
    {
        this->debug      = debug;
        this->osc_sender = osc_sender;
    }

    ~SerialConnector()
    { /* empty */
    }
    void addToMessageQueue(serial_queue_entry_t* message);

protected:
    TSQueue<serial_queue_entry_t*> ts_message_queue;
    bool                           debug = false;
    OscSender*                     osc_sender;
    void                           threadLoop();

private:
    static void* InternalThreadEntryFunc(void* This)
    {
        ((SerialConnector*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __SERIAL_CONNECTOR__