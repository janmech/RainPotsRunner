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
#include <iostream>
#include "../TheadClass/ThreadClass.h"


class OscListener : public ThreadClass
{
public:
    OscListener()
    { /* empty */
    }

    ~OscListener()
    { /* empty */
    }

protected:
    void threadLoop();

private:
    static void *
    InternalThreadEntryFunc(void *This)
    {
        ((OscListener *)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __OSC_LISTENER__