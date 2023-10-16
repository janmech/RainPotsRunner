#include "OscSender.hpp"

void OscSender::addRNBOListenter()
{
    if (this->socket_out < 0)
    {
        try
        {
            this->openOutSocket();
        }
        catch (const char *msg)
        {
            std::cerr << "RNBO Listener not added: " << msg << std::endl;
            return;
        }
    }

    this->sendMessage( "/rnbo/listeners/add", "s", "127.0.0.1:5555");
}

void OscSender::sendMessage(const char *address, const char *format, ...)
{
    char buffer[2048];
    va_list ap;
    va_start(ap, format);
    const uint32_t len = tosc_vwrite(buffer, sizeof(buffer), address, format, ap);
    va_end(ap);
    sendto(this->socket_out, buffer, len, 0, (struct sockaddr *)&this->addr_out, sizeof(this->addr_out));
}

int OscSender::openOutSocket()
{
    this->socket_out = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->socket_out == -1)
    {
        throw "Error creating OSC out socket.";
    }
    fcntl(this->socket_out, F_SETFL, O_NONBLOCK);
    struct sockaddr_in out_addr;
    out_addr.sin_family = AF_INET;
    out_addr.sin_port = htons(1234);
    this->addr_out = out_addr;
    int res = inet_pton(AF_INET, "127.0.0.1", &out_addr.sin_addr);
    printf("%d\n", res);

    bind(this->socket_out, (struct sockaddr *)&this->addr_out, sizeof(struct sockaddr_in));
    return this->socket_out;
}
