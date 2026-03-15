#ifndef __PINGER__
#define __PINGER__
#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#ifdef __cplusplus
}
#endif

#include "../TheadClass/ThreadClass.hpp"
#include "../bash_colors.hpp"
#include "../data/TSQueue.hpp"
#include "../rainpot_types.hpp"
#include "SerialConnector.hpp"

class Pinger : public ThreadClass {
public:
    Pinger(bool debug = false) { this->debug = debug; }

    ~Pinger()
    { /* empty */
    }

    void setSerialConnector(SerialConnector * ser_conn);

protected:
    bool debug = false;
    void threadLoop();

private:
    SerialConnector * ptr_serial_connector = nullptr;


    static void* InternalThreadEntryFunc(void* This)
    {
        ((Pinger*)This)->threadLoop();
        return NULL;
    }

    pthread_t _thread;
};

#endif // __PINGER__
