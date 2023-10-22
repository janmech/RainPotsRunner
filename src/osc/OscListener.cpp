#include "OscListener.hpp"

void OscListener::threadLoop()
{
    char buffer[2048];
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5555);
    sin.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
    while (this->keep_running)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);
        struct timeval timeout = {1, 0}; // select times out after 1 second
        if (select(fd + 1, &readSet, NULL, NULL, &timeout) > 0)
        {
            struct sockaddr sa; // can be safely cast to sockaddr_in
            socklen_t sa_len = sizeof(struct sockaddr_in);
            int len = 0;
            while ((len = (int)recvfrom(fd, buffer, sizeof(buffer), 0, &sa, &sa_len)) > 0)
            {
                if (tosc_isBundle(buffer))
                {
                    tosc_bundle bundle;
                    tosc_parseBundle(&bundle, buffer, len);
                    tosc_message osc;
                    while (tosc_getNextMessage(&bundle, &osc))
                    {
                        tosc_printMessage(&osc);
                    }
                }
                else
                {
                    tosc_message osc;
                    tosc_parseMessage(&osc, buffer, len);
                    // tosc_printMessage(&osc);
                    std::string address(tosc_getAddress(&osc));
                    if (address == "/rnbo/inst/0/presets/load")
                    {
                        if (this->debug)
                        {
                            std::cout << "\n\nSTART LOADING\n"
                                      << std::endl;
                        }

                        this->data_handler->clearPathValues();
                        this->data_handler->setCollectValues(true);
                    }
                    if (address == "/rnbo/inst/0/presets/loaded")
                    {
                        if (this->debug)
                        {
                            std::cout << "\n\nFINISHED LOADING\n"
                                      << std::endl;
                        }
                        this->data_handler->setCollectValues(false);
                        this->data_handler->printPathValues();
                    }

                    std::string suffix = "normalized";

                    if (
                        this->data_handler->getCollectValues() && std::mismatch(suffix.rbegin(), suffix.rend(), address.rbegin()).first == suffix.rend())
                    {
                        float value = tosc_getNextFloat(&osc);
                        this->data_handler->setPathValue(address, value);
                    };
                }
            }
        }
        usleep(THREAD_LOOP_SLEEP_US);
    }

    // close the UDP socket
    close(fd);
    if (this->debug)
    {
        printf("\tClosing UDP socket\n");
        std::cout << "\tOscListener Terminated" << std::endl;
    }
}