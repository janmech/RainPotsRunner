#include "OscSender.hpp"

void OscSender::threadLoop()
{
    while (this->keep_running)
    {
        // printf("OscSender::run....%d\n", this->inc);
        while (this->message_queue.size() > 0)
        {
            queue_entry_message_t *msg = this->message_queue.front();
            this->message_queue.pop_front();
            printf("OSC MESSAGE: ");
            for (size_t i = 0; i < msg->buffer_size; i++)
            {
                printf("0x%02X ", msg->buffer[i]);
            }
            printf("\n");
            msg_osc_t message_data;
            this->getOscMessageData(msg, &message_data);
            if (message_data.path != "")
            {
                this->sendMessage(
                    message_data.path.c_str(),
                    message_data.format.c_str(),
                    message_data.val_float);
            }
        }
    }
    close(this->socket_out);
    if (this->debug)
    {
        std::cout << "\tOscSender Terminated" << std::endl;
    }
}

void OscSender::addToMessageQueue(queue_entry_message_t *message)
{
    this->message_queue.push_back(message);
}

void OscSender::getOscMessageData(queue_entry_message_t *queue_message, msg_osc_t *osc_message_data)
{
    osc_message_data->unit = (int)(queue_message->buffer[0] & 0x0F);
    osc_message_data->controller = (int)queue_message->buffer[1];
    switch (queue_message->type)
    {
    case OSC_MESSAGE_TYPE_CC:
        osc_message_data->format = "i";
        // TODO: Normalize properly and condider center config
        osc_message_data->val_float = (float)((queue_message->buffer[3] << 7) | queue_message->buffer[2]) / 511.;
        break;
    // TODO: implement save and load messages
    // case OSC_MESSAGE_TYPE_PRESET_LOAD:
    // case OSC_MESSAGE_TYPE_PRESET_SAVE:
    //     osc_message_data->format = "f";
    //     break;
    default:
        // do nothing
        break;
    }
    osc_message_data->path = this->data_handler->getPathForController(
        osc_message_data->unit,
        osc_message_data->controller);
}

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

    this->sendMessage("/rnbo/listeners/add", "s", "127.0.0.1:5555");
}

void OscSender::sendMessage(const char *address, const char *format, ...)
{
    if (this->debug)
    {
        std::string addr_string = std::string(address);
        std::cout << "SENDING OSC MESSGSE TO " << addr_string << std::endl;
    }
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
