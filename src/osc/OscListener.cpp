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
        std::cout << "listener running" << std::endl;
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
                printf("got message\n");
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
                    tosc_printMessage(&osc);
                }
            }
        }
    }

    // close the UDP socket
    close(fd);
    printf("\tClosing UDP socket\n");
    std::cout << "\tOscListener Terminated" << std::endl;
}