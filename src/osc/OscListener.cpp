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
        struct timeval timeout = { 0, 100000 }; // select times out after 100 ms

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
                    if (this->debug && address != "/rnbo/jack/info/cpu_load") {
                        std::string format = tosc_getFormat(&osc);
                        std::cout << BACO_CYAN << "--> Incoming OSC message from: " << BACO_GRAY << address << " [" << format << "]"
                                  << BACO_END << std::endl;
                    }

                    //
                    std::regex  pattern(R"(^/rnbo/inst/(\d+)/(.*))");
                    std::smatch match;
                    std::string instance_index   = "";
                    std::string instance_message = "";

                    if (std::regex_match(address, match, pattern)) {
                        if (match.size() >= 3) {
                            instance_index   = match[1];
                            instance_message = match[2];
                        }
                    }

                    if (address == "/rnbo/resp") {
                        std::string response = std::string(tosc_getNextString(&osc));
                        if (this->debug) {
                            std::cout << "\t" << BACO_GRAY << response << BACO_END << std::endl;
                        }
                        try {
                            Json::Value reponse = this->data_handler->parseStringToJSON(response);
                            if (reponse["result"]["code"].asInt() == 2
                                && std::string(reponse["result"]["message"].asString()) == "loaded"
                                && reponse["result"]["progress"].asInt() == 100) {
                                this->data_handler->getParams(true);
                                if (this->debug) {
                                    this->data_handler->printParamConfig();
                                }
                            }
                        } catch (...) {
                            std::cerr << "Error parsing JSON String";
                        }
                    }

                    if (instance_message == "messages/out/meter") {
                        float meter_index = tosc_getNextFloat(&osc);
                        float meter_value = tosc_getNextFloat(&osc) * 255.;

                        char msg_buffer[4] = { 0xF0, // Start Condition - by convetion RainPotMeterModule is always at index 0
                                               0xE6, // Remote Message: Set Meter
                                               (char)meter_index, (char)meter_value };

                        serial_queue_entry_t serial_queue_entry;
                        // serial_queue_entry.buffer      = msg_buffer;
                        std::copy(std::begin(msg_buffer), std::end(msg_buffer), std::begin(serial_queue_entry.buffer));
                        serial_queue_entry.buffer_size = 4;
                        this->serial_connector->addToMessageQueue(&serial_queue_entry);
                    }

                    if (address == "/rnbo/inst/control/load" || address == "/rnbo/inst/control/unload") {
                        this->patcher_load_received = true;
                    }

                    if (address == "/rnbo/inst/control/sets/meta" && this->patcher_load_received) {
                        /* When loading immediatle the presets don't show up. Give  RNBO time to finish writing the config file*/
                        // TODO: See if there is a more solid solution. This might fail patches with a lot of params. Maybe ask
                        // Alex Normann.
                        usleep(200 * 1000);
                        this->data_handler->getParams(true);
                        this->patcher_load_received = false;
                        if (this->debug) {
                            this->data_handler->printParamConfig();
                        }
                    }

                    if (address == "/rnbo/inst/control/sets/load") {
                        std::string set_name = std::string(tosc_getNextString(&osc));
                        if (this->debug) {
                            std::cout << "\n\n" << "Loading Set/Graph: " << set_name << "\n\n" << std::endl;
                        }
                    }

                    if (address == "/rnbo/inst/control/sets/presets/load") {
                        std::string preset_name = std::string(tosc_getNextString(&osc));
                        if (this->debug) {
                            std::cout << BACO_GRAY "<-> Start loading preset: " << preset_name << BACO_END << std::endl << std::endl;
                            std::cout << BACO_GRAY "<-> Start loading params" << BACO_END << std::endl << std::endl;
                        }

                        this->data_handler->clearPathValues();
                        this->data_handler->setCollectValues(true);
                    }

                    if (address == "/rnbo/inst/control/sets/presets/loaded") {
                        this->data_handler->setCollectValues(false);
                        std::string preset_name = std::string(tosc_getNextString(&osc));
                        this->setRainPotsButtons(preset_name);
                        if (this->debug) {
                            std::cout << BACO_GRAY "<-> Finished loading preset: " << preset_name << BACO_END << std::endl
                                      << std::endl;
                        }
                    }

                    std::string suffix = "normalized";

                    // TODO: I need to redo this and parse the entire rainpot config

                    if (address.starts_with("/rnbo/inst/") && address.ends_with("/meta")) {
                        std::string meta_data_string = tosc_getNextString(&osc);
                        // TODO: clear config when meta string is empty or has no "rainpots" key
                        std::string path = address.substr(0, address.find("meta")) + "normalized";
                        try {
                            if (meta_data_string == "") {
                                this->data_handler->unassignControllerByPath(path);
                            } else {
                                this->data_handler->updateMetaData(meta_data_string, path);
                            }

                        } catch (...) {
                            if (this->debug) {
                                this->data_handler->unassignControllerByPath(path);
                                std::cerr << BACO_RED << "Error parsing meta data as JSON for: " << address << BACO_END << std::endl;
                                std::exception_ptr p = std::current_exception();
                                std::cout << (p ? p.__cxa_exception_type()->name() : "null") << std::endl;
                                this->data_handler->getParams(true);
                            }
                        }
                    }

                    if (this->data_handler->getCollectValues()
                        && std::mismatch(suffix.rbegin(), suffix.rend(), address.rbegin()).first == suffix.rend()) {
                        float value = tosc_getNextFloat(&osc);
                        this->data_handler->setPathValue(address, value);
                    };

                    // set button states and pickup values
                    if (address.ends_with("/normalized")) {
                        if (!this->data_handler->getCollectValues()) {
                            int unit      = -1;
                            int ctl_index = -1;
                            if (this->data_handler->getUnitCtlIndexFromPath(address, unit, ctl_index)) {
                                float value = tosc_getNextFloat(&osc);
                                this->data_handler->setPathValue(address, value);
                                if (ctl_index < 6) {
                                    serial_queue_entry_t entry = this->data_handler->makeSetButtonMessage(unit, ctl_index, address);
                                    this->serial_connector->addToMessageQueue(&entry);
                                }
                            };
                        }
                    }
                }
            }
        }
    }

    // close the UDP socket
    close(fd);
    if (this->debug) {
        std::cout << "\tOscListener: UDP socket closed" << std::endl;
        std::cout << "\tOscListener Terminated" << std::endl << std::endl;
    }
}

void OscListener::setRainPotsButtons(std::string preset_name)
{
    if (this->debug) {
        std::cout << std::endl << BACO_GRAY << "<-> Finished loading preset: " << preset_name << BACO_END << std::endl << std::endl;
        std::cout << std::endl << BACO_GRAY << "<-> Finished loading params" << BACO_END << std::endl << std::endl;
    }
    if (this->debug) {
        this->data_handler->printPathValues();
    }
    std::map<int, serial_queue_entry_t>           ser_messages     = this->data_handler->makeSetButtonValueMessages();
    std::map<int, serial_queue_entry_t>::iterator ser_msg_iterator = ser_messages.begin();

    while (ser_msg_iterator != ser_messages.end()) {
        this->serial_connector->addToMessageQueue(&ser_msg_iterator->second);
        ser_msg_iterator++;
    }

    try {
        std::string::size_type sz;
        volatile int           preset_name_mumeric = std::stoi(preset_name, &sz);
        preset_name_mumeric                        = (preset_name_mumeric < 0) ? 0 : preset_name_mumeric;
        preset_name_mumeric                        = (preset_name_mumeric > 99) ? 99 : preset_name_mumeric;

        char index_byte = (char)preset_name_mumeric;

        char buff_msg_set_preset[4] = { (char)0xF0, (char)0xE8, index_byte, (char)0x00 };
        // For now we set the RainPot Indicator to "loaded" when loading starts.
        // Later if implemente in RNBO we do this in two steps

        serial_queue_entry_t msg_set_preset;

        std::copy(std::begin(buff_msg_set_preset), std::end(buff_msg_set_preset), std::begin(msg_set_preset.buffer));
        msg_set_preset.buffer_size = 4;

        this->serial_connector->addToMessageQueue(&msg_set_preset);
        usleep(1000);

    } catch (...) {
        if (this->debug) {
            std::cerr << BACO_RED << "Loaded preset name cannot be tranlated to a number" << BACO_END << std::endl;
        }
    }
}
