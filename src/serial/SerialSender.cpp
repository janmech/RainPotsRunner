#include "SerialSender.hpp"

void SerialSender::threadLoop()
{

    while (this->p_fd == NULL) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    while (this->keep_running) {
        serial_queue_entry_t* entry = this->ptr_serial_connector->popFromMessageQueue();

        // FIXME: This is a work around because sometimes buffersize is negative (uninitialized?). Find real cause and fix it.
        if (entry->buffer_size > 0) {
            write(*this->p_fd, entry->buffer, entry->buffer_size);
            if (this->debug) {
                std::cout << BACO_MAGENTA << "<-- Sending serial packet: [" << entry->buffer_size << "] " << BACO_END;
                std::cout << BACO_GRAY;
                for (int i = 0; i < entry->buffer_size; i++) {
                    printf("0x%02X ", entry->buffer[i]);
                }
                std::cout << BACO_END << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }
    if (this->debug) {
        std::cout << "\tSerialSender Terminated" << std::endl << std::endl;
    }
}

void SerialSender::setFileDescriptor(int* fd) { this->p_fd = fd; }

void SerialSender::setSerialConnector(SerialConnector* ser_conn) { this->ptr_serial_connector = ser_conn; }
