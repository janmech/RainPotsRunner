#include "OscListener.hpp"

void OscListener::threadLoop()
{
    char      buffer[2048];
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
    struct sockaddr_in sin;
    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(5555);
    sin.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
    while (this->keep_running) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);
        struct timeval timeout = { 1, 0 }; // select times out after 1 second
        if (select(fd + 1, &readSet, NULL, NULL, &timeout) > 0) {
            struct sockaddr sa; // can be safely cast to sockaddr_in
            socklen_t       sa_len = sizeof(struct sockaddr_in);
            int             len    = 0;
            while ((len = (int)recvfrom(fd, buffer, sizeof(buffer), 0, &sa, &sa_len)) > 0) {
                if (tosc_isBundle(buffer)) {
                    tosc_bundle bundle;
                    tosc_parseBundle(&bundle, buffer, len);
                    tosc_message osc;
                    while (tosc_getNextMessage(&bundle, &osc)) {
                        if (this->debug) {
                            tosc_printMessage(&osc);
                        }
                        std::cerr << "OSC Bundels are currently not supported" << std::endl;
                    }
                } else {
                    tosc_message osc;
                    tosc_parseMessage(&osc, buffer, len);
                    std::string address(tosc_getAddress(&osc));
                    if (this->debug) {
                        std::string format = tosc_getFormat(&osc);
                        std::cout << BACO_CYAN << "--> Incoming OSC message from: " << BACO_GRAY << address << " [" << format << "]" << BACO_END << std::endl;
                    }
                    if (address == "/rnbo/resp") {
                        std::string response = std::string(tosc_getNextString(&osc));
                        if (this->debug) {
                            std::cout << "\t" << BACO_GRAY << response << BACO_END << std::endl;
                        }
                        try {
                            Json::Value reponse = this->data_handler->parseStringToJSON(response);
                            if (
                                reponse["result"]["code"].asInt() == 2
                                && std::string(reponse["result"]["message"].asString()) == "loaded"
                                && reponse["result"]["progress"].asInt() == 100

                            ) {
                                this->data_handler->getParams(true);
                                if (this->debug) {
                                    this->data_handler->printParamConfig();
                                }
                            }
                        } catch (...) {
                            std::cerr << "Error parsing JSON String";
                        }
                    }
                    if (address == "/rnbo/inst/0/messages/out/meter") {
                        float meter_index = tosc_getNextFloat(&osc);
                        float meter_value = tosc_getNextFloat(&osc) * 255.;

                        char msg_buffer[4] = {
                            0xF0, // Start Condition - by convetion RainPotMeterModule is always at index 0
                            0xE6, // Remote Message: Set Meter
                            (char)meter_index,
                            (char)meter_value
                        };

                        serial_queue_entry_t serial_queue_entry;
                        serial_queue_entry.buffer      = msg_buffer;
                        serial_queue_entry.buffer_size = 4;
                        this->serial_connector->addToMessageQueue(&serial_queue_entry);
                    }

                    if (address == "/rnbo/inst/control/load") {
                        this->patcher_load_received = true;
                    }

                    if (address == "/rnbo/inst/0/name" && this->patcher_load_received) {
                        this->data_handler->getParams(true);
                        this->patcher_load_received = false;
                        if (this->debug) {
                            this->data_handler->printParamConfig();
                        }
                    }

                    if (address == "/rnbo/inst/0/presets/load") {
                        if (this->debug) {
                            std::cout << BACO_GRAY "<-> Start loading params" << BACO_END << std::endl
                                      << std::endl;
                        }

                        this->data_handler->clearPathValues();
                        this->data_handler->setCollectValues(true);
                    }

                    if (address == "/rnbo/inst/0/presets/loaded") {
                        if (this->debug) {
                            std::cout << std::endl
                                      << BACO_GRAY << "<-> Finished loading params" << BACO_END << std::endl
                                      << std::endl;
                        }
                        this->data_handler->setCollectValues(false);
                        if (this->debug) {
                            this->data_handler->printPathValues();
                        }
                    }

                    std::string suffix = "normalized";

                    if (
                        this->data_handler->getCollectValues()
                        && std::mismatch(suffix.rbegin(), suffix.rend(), address.rbegin()).first == suffix.rend()) {
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
    if (this->debug) {
        std::cout << "\tOscListener: UDP socket closed" << std::endl;
        std::cout << "\tOscListener Terminated" << std::endl
                  << std::endl;
    }
}