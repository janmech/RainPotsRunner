// Serial connection is working thanks to this wonderful blog post:
// https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

#ifndef __SERIAL_CONNECTOR__
#define __SERIAL_CONNECTOR__
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string_view>
// Linux headers
#include <asm/termios.h> // Contains POSIX terminal control definitions
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <unistd.h> // write(), read(), close()
namespace fixioctl {
#include <sys/ioctl.h>
}

#include "../TheadClass/ThreadClass.h"
#include "../bash_colors.hpp"
#include "../data/DataHandler.hpp"
#include "../data/TSQueue.hpp"
#include "../osc/OscSender.hpp"
#include "../rainpot_types.hpp"

#define SERIAL_PORT_PATH     "/dev/ttyS0"
#define SERIAL_IN_BUFFER_LEN 100

#define MSG_BUFFER_SIZE         100
#define MSG_CC_STATUS_BIT_MASK  0x0B
#define MSG_CC_PACKET_SIZE      4
#define MSG_PGM_STATUS_BIT_MASK 0x0C
#define MSG_PGM_PACKET_SIZE     2
#define MSG_SAVE                0xF4
#define MSG_SAVE_PACKET_SIZE    2

class SerialConnector : public ThreadClass {
public:
    SerialConnector(OscSender* osc_sender, DataHandler* data_handler, bool debug = false)
    {
        this->debug        = debug;
        this->osc_sender   = osc_sender;
        this->data_handler = data_handler;
    }

    ~SerialConnector()
    { /* empty */
    }
    void addToMessageQueue(serial_queue_entry_t* message);

protected:
    TSQueue<serial_queue_entry_t*> ts_message_queue;
    bool                           debug = false;
    OscSender*                     osc_sender;
    DataHandler*                   data_handler;
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