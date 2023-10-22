#include "OscSender.hpp"

void OscSender::threadLoop()
{
    while (this->keep_running) {
        if (this->ts_message_queue.size() > 0) {
            queue_entry_message_t* msg = this->ts_message_queue.pop();
            if (this->debug) {
                std::cout << BACO_GRAY << "<-> Processing OSC message: " << BACO_END;
                for (size_t i = 0; i < msg->buffer_size; i++) {
                    printf("0x%02X ", msg->buffer[i]);
                }
                std::cout << std::endl;
            }
            msg_osc_t message_data;
            this->getOscMessageData(msg, &message_data);
            if (message_data.path != "") {
                const char* path   = message_data.path.c_str();
                const char* format = message_data.format.c_str();
                if (this->debug) {
                    std::cout << BACO_MAGENTA << "<-- Sending OSC message to: " << BACO_END << std::endl
                              << "\t" << path << std::endl
                              << "\t" << format << std::endl;
                    if (format[0] == 'f') {
                        std::cout << "\t" << message_data.val_float << std::endl;
                    } else if (format[0] == 's') {
                        std::cout << "\t" << message_data.val_string << std::endl;
                    } else {
                        std::cout << "\t usupported format:" << format << std::endl;
                    }
                }
                switch (msg->type) {
                case OSC_MESSAGE_TYPE_CC:
                    this->sendMessage(
                        path,
                        format,
                        message_data.val_float);
                    break;
                case OSC_MESSAGE_TYPE_PRESET_LOAD:
                case OSC_MESSAGE_TYPE_PRESET_SAVE: {
                    const char* string_val = message_data.val_string.c_str();
                    this->sendMessage(
                        path,
                        format,
                        string_val);
                } break;

                default:
                    std::cerr << "OscSender: Invalid mesage type" << std::endl;
                    break;
                }
            }
        }
        usleep(THREAD_LOOP_SLEEP_US);
    }

    close(this->socket_out);
    if (this->debug) {
        std::cout << "\tOscSender: UDP socket closed" << std::endl;
        std::cout << "\tOscSender Terminated" << std::endl
                  << std::endl;
    }
}

void OscSender::addToMessageQueue(queue_entry_message_t* message)
{
    this->ts_message_queue.push(message);
}

void OscSender::getOscMessageData(queue_entry_message_t* queue_message, msg_osc_t* osc_message_data)
{
    osc_message_data->unit       = (int)(queue_message->buffer[0] & 0x0F);
    osc_message_data->controller = (int)queue_message->buffer[1];
    switch (queue_message->type) {
    case OSC_MESSAGE_TYPE_CC: {
        int raw_int_value           = (int)((queue_message->buffer[3] << 7) | queue_message->buffer[2]);
        osc_message_data->val_float = this->data_handler->makeValueFLoat(
            osc_message_data->unit,
            osc_message_data->controller,
            raw_int_value);
        osc_message_data->format = "f";
        osc_message_data->path   = this->data_handler->getPathForController(
            osc_message_data->unit,
            osc_message_data->controller);
    } break;
    case OSC_MESSAGE_TYPE_PRESET_SAVE: {
        osc_message_data->format = "s";
        osc_message_data->path   = "/rnbo/inst/0/presets/save";
        std::stringstream preset_name;
        preset_name << std::setw(3) << std::setfill('0') << (int)queue_message->buffer[1];
        std::cout << preset_name.str() << std::endl;
        osc_message_data->val_string = preset_name.str();
    } break;
    case OSC_MESSAGE_TYPE_PRESET_LOAD: {
        osc_message_data->format = "s";
        osc_message_data->path   = "/rnbo/inst/0/presets/load";
        std::stringstream preset_name;
        preset_name << std::setw(3) << std::setfill('0') << (int)queue_message->buffer[1];
        std::cout << preset_name.str() << std::endl;
        osc_message_data->val_string = preset_name.str();
    } break;
    default:
        std::cerr << "OscSender: Invalid Message Type: " << queue_message->type << std::endl;
        break;
    }
}

void OscSender::addRNBOListenter()
{

    this->sendMessage("/rnbo/listeners/add", "s", "127.0.0.1:5555");
}

void OscSender::sendMessage(const char* address, const char* format, ...)
{
    if (this->socket_out < 0) {
        try {
            this->openOutSocket();
        } catch (const char* msg) {
            std::cerr << "Erros sending message: " << msg << std::endl;
            return;
        }
    }
    char    buffer[2048];
    va_list ap;
    va_start(ap, format);
    const uint32_t len = tosc_vwrite(buffer, sizeof(buffer), address, format, ap);
    va_end(ap);
    sendto(this->socket_out, buffer, len, 0, (struct sockaddr*)&this->addr_out, sizeof(this->addr_out));
}

int OscSender::openOutSocket()
{
    this->socket_out = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->socket_out == -1) {
        throw "Error creating OSC out socket.";
    }
    fcntl(this->socket_out, F_SETFL, O_NONBLOCK);
    struct sockaddr_in out_addr;
    out_addr.sin_family = AF_INET;
    out_addr.sin_port   = htons(1234);
    this->addr_out      = out_addr;
    int res             = inet_pton(AF_INET, "127.0.0.1", &out_addr.sin_addr);

    bind(this->socket_out, (struct sockaddr*)&this->addr_out, sizeof(struct sockaddr_in));
    return this->socket_out;
}
