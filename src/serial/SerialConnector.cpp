#include "SerialConnector.hpp"

void SerialConnector::threadLoop()
{
    struct termios options;

    char    serial_in_buffer[SERIAL_IN_BUFFER_LEN];
    char    msg_packet_buffer[MSG_BUFFER_SIZE];
    uint8_t msg_packet_index = 0;
    uint8_t msg_packet_size  = 0;
    uint8_t status_byte      = 0;
    bool    is_parsing       = false;
    int     msg_type         = OSC_MESSAGE_TYPE_NONE;

    int fd = open(SERIAL_PORT_PATH, O_RDWR);
    if (fd < 0) {
        std::cerr << "Error opening serial port " << SERIAL_PORT_PATH << std::endl;
        this->keep_running = false;
    }
    // Create new termios struct, we call it 'tty' for convention
    struct termios2 tty;

    // Read in existing settings, and handle any error
    fixioctl::ioctl(fd, TCGETS2, &tty);

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN]  = 0;

    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= CBAUDEX;
    tty.c_ispeed = 380400; // What a custom baud rate!
    tty.c_ospeed = 380400;

    fixioctl::ioctl(fd, TCSETS2, &tty);

    memset(serial_in_buffer, 0, SERIAL_IN_BUFFER_LEN);
    memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);

    while (this->keep_running) {
        // Read incomng data
        if (int received = read(fd, serial_in_buffer, 1) > 0) {
            if (!is_parsing) {
                // We use an undefiged System Common Messages as a status byte foe 'save', hence there is no channel information
                if (serial_in_buffer[0] == MSG_SAVE) {
                    msg_packet_size  = MSG_SAVE_PACKET_SIZE;
                    msg_packet_index = 0;
                    is_parsing       = true;
                    memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                    msg_type = OSC_MESSAGE_TYPE_PRESET_SAVE;
                } else {
                    status_byte = serial_in_buffer[0] >> 4;
                    switch (status_byte) {
                    case MSG_CC_STATUS_BIT_MASK:
                        msg_packet_size  = MSG_CC_PACKET_SIZE;
                        msg_packet_index = 0;
                        is_parsing       = true;
                        msg_type         = OSC_MESSAGE_TYPE_CC;
                        memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                        break;
                    case MSG_PGM_STATUS_BIT_MASK:
                        msg_packet_size  = MSG_PGM_PACKET_SIZE;
                        msg_packet_index = 0;
                        is_parsing       = true;
                        msg_type         = OSC_MESSAGE_TYPE_PRESET_LOAD;
                        memset(msg_packet_buffer, 0, MSG_BUFFER_SIZE);
                        break;
                    default:
                        break;
                    }
                }
            }
            if (is_parsing) {
                msg_packet_buffer[msg_packet_index] = serial_in_buffer[0];
                msg_packet_index++;
            }
            if (msg_packet_index == msg_packet_size) {
                if (this->debug) {
                    std::cout << BACO_CYAN << "--> Serial package parsed: " << BACO_END;
                    for (size_t i = 0; i < msg_packet_size; i++) {
                        printf("0x%02X ", msg_packet_buffer[i]);
                    }
                    std::cout << std::endl;
                }
                msg_packet_size  = 0;
                msg_packet_index = 0;
                is_parsing       = false;
                queue_entry_message_t msg;
                msg.type   = msg_type;
                msg.buffer = msg_packet_buffer;
                switch (msg_type) {
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
                if (msg.type != OSC_MESSAGE_TYPE_NONE) {
                    if (msg.type == OSC_MESSAGE_TYPE_CC) {
                        serial_queue_entry_t pick_up_message;

                        char pickup_msg_buffer[]    = { 0x00, 0x00, 0x00 };
                        pick_up_message.buffer      = pickup_msg_buffer;
                        pick_up_message.buffer_size = 3;
                        int pick_up_action          = this->data_handler->makeValuePickupMessasge(&msg, &pick_up_message);
                        this->addToMessageQueue(&pick_up_message);
                        
                        if (pick_up_action == PICK_UP_LOCKED) {
                            this->osc_sender->addToMessageQueue(&msg);
                        } else {
                            if (this->debug) {
                                std::cout << BACO_YELLO << "Param neads to be picked up..." << BACO_END << std::endl;
                            }
                        }

                    } else {
                        this->osc_sender->addToMessageQueue(&msg);
                    }
                }
                msg_type = OSC_MESSAGE_TYPE_NONE;
            }
        };

        // Send data from message queue
        if (this->ts_message_queue.size() > 0) {
            serial_queue_entry_t* entry = this->ts_message_queue.pop();

            int bytes_written = write(fd, entry->buffer, entry->buffer_size);

            if (this->debug) {
                std::cout << BACO_MAGENTA << "<-- Sending serial packet: " << BACO_END;
                std::cout << BACO_GRAY;
                for (size_t i = 0; i < entry->buffer_size; i++) {
                    printf("0x%02X ", entry->buffer[i]);
                }
                std::cout << BACO_END << std::endl;
            }
        }

        usleep(THREAD_LOOP_SLEEP_US);
    }
    close(fd);
    if (this->debug) {
        std::cout << "\tSerialConnector: Serial connection closed." << std::endl;
        std::cout << "\tSerialConnector Terminated" << std::endl << std::endl;
    }
}

void SerialConnector::addToMessageQueue(serial_queue_entry_t* message) { this->ts_message_queue.push(message); }
