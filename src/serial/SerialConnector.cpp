#include "SerialConnector.hpp"

void SerialConnector::threadLoop()
{
    struct termios options;

    char serial_in_buffer[SERIAL_IN_BUFFER_LEN];
    char msg_packet_buffer[MSG_BUFFER_SIZE];
    uint8_t msg_packet_index = 0;
    uint8_t msg_packet_size = 0;
    uint8_t status_byte = 0;
    bool is_parsing = false;
    int msg_type = OSC_MESSAGE_TYPE_NONE;

    int fd = open(SERIAL_PORT_PATH, O_RDWR | O_NDELAY);

    tcgetattr(fd, &options);

    cfmakeraw(&options);
    cfsetospeed(&options, B230400);
    options.c_cc[VMIN] = 2;
    options.c_cc[VTIME] = 1;
    options.c_cflag &= ~CRTSCTS;
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    if (tcsetattr(fd, TCSANOW, &options) == -1)
    {
        std::cout << "tcsetattr failed" << std::endl;
        this->keep_running = false;
    }
    tcflush(fd, TCIOFLUSH); // Clear IO buffer

    memset(serial_in_buffer, 0, SERIAL_IN_BUFFER_LEN);
    memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);

    while (this->keep_running)
    {
        if (int received = read(fd, serial_in_buffer, 1) > 0)
        {
            if (!is_parsing)
            {
                // We use an undefiged System Common Messages as a status byte foe 'save', hence there is no channel information
                if (serial_in_buffer[0] == MSG_SAVE)
                {
                    msg_packet_size = MSG_SAVE_PACKET_SIZE;
                    msg_packet_index = 0;
                    is_parsing = true;
                    memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                    msg_type = OSC_MESSAGE_TYPE_PRESET_SAVE;
                }
                else
                {
                    status_byte = serial_in_buffer[0] >> 4;
                    switch (status_byte)
                    {
                    case MSG_CC_STATUS_BIT_MASK:
                        msg_packet_size = MSG_CC_PACKET_SIZE;
                        msg_packet_index = 0;
                        is_parsing = true;
                        msg_type = OSC_MESSAGE_TYPE_CC;
                        memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                        break;
                    case MSG_PGM_STATUS_BIT_MASK:
                        msg_packet_size = MSG_PGM_PACKET_SIZE;
                        msg_packet_index = 0;
                        is_parsing = true;
                        msg_type = OSC_MESSAGE_TYPE_PRESET_LOAD;
                        memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                        break;
                    default:
                        break;
                    }
                }
            }
            if (is_parsing)
            {
                msg_packet_buffer[msg_packet_index] = serial_in_buffer[0];
                msg_packet_index++;
            }
            if (msg_packet_index == msg_packet_size)
            {
                if (this->debug)
                {
                    printf("Serial Package: ");
                    for (size_t i = 0; i < msg_packet_size; i++)
                    {
                        printf("0x%02X ", msg_packet_buffer[i]);
                    }
                    printf("\n");
                }
                msg_packet_size = 0;
                msg_packet_index = 0;
                is_parsing = false;
                queue_entry_message_t msg;
                msg.type = msg_type;
                msg.buffer = msg_packet_buffer;
                switch (msg_type)
                {
                case OSC_MESSAGE_TYPE_CC:
                    msg.buffer_size = MSG_CC_PACKET_SIZE;
                    break;
                case OSC_MESSAGE_TYPE_PRESET_LOAD:
                    msg.buffer_size = MSG_PGM_PACKET_SIZE;
                    break;
                case OSC_MESSAGE_TYPE_PRESET_SAVE:
                    msg.buffer_size = MSG_SAVE_PACKET_SIZE;
                    break;
                default:
                    msg.type = OSC_MESSAGE_TYPE_NONE;
                    break;
                }
                if (msg.type != OSC_MESSAGE_TYPE_NONE)
                {
                    this->osc_sender->addToMessageQueue(&msg);
                }
                msg_type = OSC_MESSAGE_TYPE_NONE;
            }
        };
        usleep(100);
    }
    close(fd);
    if (this->debug)
    {
        std::cout << "\tSerialConnector Terminated" << std::endl;
    }
}