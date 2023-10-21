#ifndef __OSC_LISTENER__
#define __OSC_LISTENER__
#ifdef __cplusplus
extern "C"
{
#endif

#include "tinyosc/tinyosc.h"
#ifdef __cplusplus
}
#endif
#include <string>
#include <string_view>
#include <iostream>
#include "../TheadClass/ThreadClass.h"
#include "../data/DataHandler.hpp"

class OscListener : public ThreadClass
{
public:
    OscListener(DataHandler *data_handler)
    {
        this->data_handler = data_handler;
    }

    ~OscListener()
    { /* empty */
    }

protected:
    DataHandler *data_handler;
    void threadLoop();

private:
    static void *InternalThreadEntryFunc(void *This)
    {
        ((OscListener *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_LISTENER__