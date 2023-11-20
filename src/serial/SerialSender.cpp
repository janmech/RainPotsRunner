#include "SerialSender.hpp"

void SerialSender::threadLoop()
{

    while (this->p_fd == NULL) {
        usleep(100 * 1000);
    }
    while (this->keep_running) {
        serial_queue_entry_t* entry = this->ptr_ts_message_queue->pop();

        int bytes_written = write(*this->p_fd, entry->buffer, entry->buffer_size);

        if (this->debug) {
            std::cout << BACO_MAGENTA << "<-- Sending serial packet: " << BACO_END;
            std::cout << BACO_GRAY;
            for (size_t i = 0; i < entry->buffer_size; i++) {
                printf("0x%02X ", entry->buffer[i]);
            }
            std::cout << BACO_END << std::endl;
        }
    }
    if (this->debug) {
        std::cout << "\tSerialSender Terminated" << std::endl << std::endl;
    }
}

void SerialSender::setFileDescriptor(int* fd) { this->p_fd = fd; }

void SerialSender::setMessageQueue(TSQueue<serial_queue_entry_t*>* ts_message_queue) { this->ptr_ts_message_queue = ts_message_queue; }
