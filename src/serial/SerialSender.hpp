#ifndef __SERIAL_SENDER__
#define __SERIAL_SENDER__
#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#ifdef __cplusplus
}
#endif

#include "../TheadClass/ThreadClass.h"
#include "../bash_colors.hpp"
#include "../data/TSQueue.hpp"
#include "../rainpot_types.hpp"

class SerialSender : public ThreadClass {
public:
    SerialSender(bool debug = false) { this->debug = debug; }

    ~SerialSender()
    { /* empty */
    }

    void setFileDescriptor(int* fd);
    void setMessageQueue(TSQueue<serial_queue_entry_t*>* ts_message_queue);

protected:
    bool debug = false;
    void threadLoop();

private:
    int*                            p_fd                 = NULL;
    TSQueue<serial_queue_entry_t*>* ptr_ts_message_queue = NULL;

    static void* InternalThreadEntryFunc(void* This)
    {
        ((SerialSender*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __SERIAL_SENDER__
