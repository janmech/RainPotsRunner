#include "osc_sender.h"

void *osc_sender_runner(void *arg)
{

    bool *keep_running = (bool *)arg;
    while (*keep_running)
    {
        printf("osc_sender_runner\n");
        sleep(1);
    }
    printf("\tosc_sender_runner Terminated\n");
    return NULL;
}

// OscSender::OscSender(std::string somnething = "jannni")
// {
//     this->foo = somnething;
// }

// void *OscSender::runner(void *arg)
// {
//     bool *keep_running = (bool *)arg;
//     while (*keep_running)
//     {
//         printf("OscSender::run.... %s\n", this->foo);
//         sleep(1);
//     }
//     printf("\tOscSender Terminated\n");
//     return NULL;
// }