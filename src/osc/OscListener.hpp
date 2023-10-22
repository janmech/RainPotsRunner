#ifndef __OSC_LISTENER__
#define __OSC_LISTENER__
#ifdef __cplusplus
extern "C" {
#endif

#include "tinyosc/tinyosc.h"
#ifdef __cplusplus
}
#endif
#include "../TheadClass/ThreadClass.h"
#include "../bash_colors.hpp"
#include "../data/DataHandler.hpp"
#include "../serial/SerialConnector.hpp"
#include <iostream>
#include <string>
#include <string_view>

class OscListener : public ThreadClass {
public:
    OscListener(DataHandler* data_handler, SerialConnector* serial_connector, bool debug = false)
    {
        this->debug            = debug;
        this->data_handler     = data_handler;
        this->serial_connector = serial_connector;
    }

    ~OscListener()
    { /* empty */
    }

protected:
    bool             debug                 = false;
    bool             patcher_load_received = false;
    DataHandler*     data_handler;
    SerialConnector* serial_connector;
    void             threadLoop();

private:
    static void* InternalThreadEntryFunc(void* This)
    {
        ((OscListener*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_LISTENER__