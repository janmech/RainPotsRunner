#include "Pinger.hpp"

void Pinger::threadLoop()
{
    while (this->keep_running) {
        if (this->ptr_serial_connector != nullptr) {
            char msg_buffer[4] = { 0xF0, // Start Condition - by convetion RainPotMeterModule is always at index 0
                                   0xE9 // Remote Message: Ping
                                  };

            serial_queue_entry_t serial_queue_entry;
            std::copy(std::begin(msg_buffer), std::end(msg_buffer), std::begin(serial_queue_entry.buffer));
            serial_queue_entry.buffer_size = 4;
            this->ptr_serial_connector->addToMessageQueue(&serial_queue_entry);
            if(this->debug) {
                 std::cout << BACO_GREEN << "---PING---" << BACO_END << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void Pinger::setSerialConnector(SerialConnector * ser_conn) {
    this->ptr_serial_connector = ser_conn;
}
