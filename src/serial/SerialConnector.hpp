#ifndef __SERIAL_CONNECTOR__
#define __SERIAL_CONNECTOR__
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __cplusplus
}
#endif
#include <string>
#include <cstring>
#include <string_view>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include "../TheadClass/ThreadClass.h"

#define SERIAL_PORT_PATH "/dev/ttyS0"
#define SERIAL_IN_BUFFER_LEN 10

#define MSG_BUFFER_SIZE 12
#define MSG_CC_STATUS_BIT_MASK 0x0B
#define MSG_CC_PACKET_SIZE 4
#define MSG_PGM_STATUS_BIT_MASK 0x0C
#define MSG_PGM_PACKET_SIZE 2
#define MSG_SAVE 0xF4
#define MSG_SAVE_PACKET_SIZE 2

class SerialConnector : public ThreadClass
{
public:
    SerialConnector()
    {
    }

    ~SerialConnector()
    { /* empty */
    }

protected:
    void threadLoop();

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((SerialConnector *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __SERIAL_CONNECTOR__