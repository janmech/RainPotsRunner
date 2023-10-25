#include "DataHandler.hpp"

config_map_t DataHandler::getParams(bool force_load)
{
    if (this->param_config.size() == 0 || force_load) {
        try {
            this->loadConfig();
        } catch (const char* msg) {
            std::cerr << msg << std::endl;
        }
    }
    return this->param_config;
}

preset_index_map_t DataHandler::getPresets(bool force_load)
{
    if (this->presets.size() == 0 || force_load) {
        try {
            this->loadConfig();
        } catch (const char* msg) {
            std::cerr << msg << std::endl;
        }
    }
    return this->presets;
}

int DataHandler::makeValuePickupMessasge(queue_entry_message_t* msg, serial_queue_entry_t* serial_queue_entry)
{
    int pick_up_action = PICK_UP_NONE;

    if (msg->type == OSC_MESSAGE_TYPE_CC && msg->buffer_size >= 4) {

        int unit       = (int)(msg->buffer[0] & 0x0F);
        int controller = (int)msg->buffer[1];
        if (controller >= 6) { // We only check for knobs, not for buttons
            if (this->contollerIsAssigned(unit, controller)) {
                std::string path = this->getPathForController(unit, controller);
                if (this->path_values.find(path) != this->path_values.end()) {

                    std::string key_unit      = std::to_string(unit);
                    std::string key_ctl       = std::to_string(controller);
                    std::string value_id      = key_unit + " " + key_ctl;
                    int         raw_int_value = (int)((msg->buffer[3] << 7) | msg->buffer[2]);
                    float       float_value   = this->makeValueFLoat(unit, controller, raw_int_value);

                    // Depending on the power supply situation the RainPots knobs can fluctiate. 
                    // Let's put a margin for the indicator leds so they dont flicker too much.
                    if (this->last_sent_ctl_values.find(value_id) != this->last_sent_ctl_values.end()) {
                        if (std::abs(this->last_sent_ctl_values[value_id] - float_value) < 0.03) {
                            return pick_up_action;
                        } else {
                            this->last_sent_ctl_values[value_id] = float_value;
                        }
                    } else {
                        this->last_sent_ctl_values.insert(std::make_pair(value_id, float_value));
                    }
                    path_value_t ctl_state = this->path_values[path];

                    if (ctl_state.locked
                        || !ctl_state.loaded) { // if not loaded we dont have information so just send the new param values
                        pick_up_action = PICK_UP_LOCKED;
                    } else if (ctl_state.loaded) {
                        // is the current value hieger or lower than the loaded one?

                        float diff = float_value - ctl_state.value;
                        if (std::abs(diff) > 0.03f) {
                            pick_up_action = (diff > 0) ? PICK_UP_TURN_DOWN : PICK_UP_TURN_UP;
                        } else {
                            pick_up_action = PICK_UP_LOCKED;
                        }
                    }
                    // 0xF0 : Start Condition. MeterModule has by convention allway index 0
                    // 0xE7 : Remote Command: signal pickup state
                    // Last Byte:
                    //         0x00 - Need to turn up
                    //         0x01 - Need to turn down
                    //         0x02 - Controller has picked up (Param is locked)
                    serial_queue_entry->buffer[0] = 0xF0;
                    serial_queue_entry->buffer[1] = 0xE7;
                    switch (pick_up_action) {
                    case PICK_UP_LOCKED:
                        // NOTE: For now we update the lock staus here. This for all practical cades should avoid a race condition
                        //       A possible case foer a race condition could be updating the config (loading a Preset) while
                        //       the knob is turned.
                        // FIXME: should be thread safe
                        if (!ctl_state.locked) {
                            ctl_state.locked        = true;
                            this->path_values[path] = ctl_state;
                        }
                        serial_queue_entry->buffer[2] = 0x02;
                        break;
                    case PICK_UP_TURN_UP:
                        serial_queue_entry->buffer[2] = 0x00;
                        break;
                    case PICK_UP_TURN_DOWN:
                        serial_queue_entry->buffer[2] = 0x01;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    return pick_up_action;
}

std::map<int, serial_queue_entry_t> DataHandler::makeSetButtonValueMessages()
{
    std::map<int, serial_queue_entry_t> serial_messages;

    config_map_t::iterator unit_iterator = this->param_config.begin();

    if (this->debug) {
        std::cout << BACO_GRAY << "<-> Collected Button Values for sending to RainPots: " << BACO_END << std::endl;
    }

    while (unit_iterator != this->param_config.end()) {
        int  unit            = unit_iterator->first;
        char start_condition = 0xF0 | (char)unit;

        char* message_buffer = new char[8];

        message_buffer[0] = start_condition;
        message_buffer[1] = (char)0xE5;
        message_buffer[2] = (char)0x0F;
        message_buffer[3] = (char)0x0F;
        message_buffer[4] = (char)0x0F;
        message_buffer[5] = (char)0x0F;
        message_buffer[6] = (char)0x0F;
        message_buffer[7] = (char)0x0F;

        serial_queue_entry_t serial_queue_entry;

        serial_queue_entry.buffer_size = 8;

        if (this->debug) {
            std::cout << BACO_GRAY << "  Unit: " << unit << BACO_END << std::endl;
        }

        std::map<int, ctl_settings_t>::iterator ctl_iterator = unit_iterator->second.begin();
        while (ctl_iterator != unit_iterator->second.end()) {

            int ctl_index = ctl_iterator->first;
            if (ctl_index < 6) { // We only cate aboyt buttons here

                ctl_settings_t ctl_settings = ctl_iterator->second;
                std::string    param_path   = ctl_settings.path;

                if (this->path_values.find(param_path) != this->path_values.end()) {
                    float normalized_value = this->path_values[param_path].value;
                    char  rainpot_value    = this->formatButtonValue(unit, ctl_index, normalized_value);

                    if (this->debug) {
                        std::cout << BACO_GRAY << "    ctl:" << ctl_index << " p:" << param_path << " norm. val:" << normalized_value
                                  << " final val: " << +rainpot_value << BACO_END << std::endl;
                    }

                    message_buffer[ctl_index + 2] = rainpot_value;
                }
            }
            ctl_iterator++;
        }
        if (this->debug) {
            std::cout << std::endl;
        }
        serial_queue_entry.buffer = message_buffer;
        serial_messages.insert(std::make_pair(unit, serial_queue_entry));
        unit_iterator++;
    }
    return serial_messages;
}

bool DataHandler::contollerIsAssigned(int unit, int controller)
{
    return (this->param_config.find(unit) != this->param_config.end())
        && (this->param_config[unit].find(controller) != this->param_config[unit].end());
}

bool DataHandler::controllerIsCentered(int unit, int controller)
{
    bool is_centered = false;
    if (this->contollerIsAssigned(unit, controller)) {
        is_centered = this->param_config[unit][controller].center;
    }
    return is_centered;
}

int DataHandler::getConrollerSteps(int unit, int controller)
{
    int steps = -1;
    if (this->contollerIsAssigned(unit, controller)) {
        steps = this->param_config[unit][controller].steps;
    }
    return steps;
}

std::string DataHandler::getPathForController(int unit, int controller)
{
    std::string osc_path = "";
    if (this->contollerIsAssigned(unit, controller)) {
        osc_path = this->param_config[unit][controller].path;
    }

    return osc_path;
}

float DataHandler::makeValueFLoat(int unit, int controler, int raw_value)
{
    float normalized_value = -1;
    float center_margin    = 0.05;
    if (this->contollerIsAssigned(unit, controler)) {
        normalized_value = (float)(raw_value) / 511.f;
        if (this->controllerIsCentered(unit, controler)) {
            if (std::abs(normalized_value - 0.5) <= center_margin) {
                normalized_value = 0.5;
            } else if (normalized_value > 0.5 + center_margin) {
                normalized_value = this->scaleValue(normalized_value, 0.5 + center_margin, 1., 0.5, 1.);
            } else if (normalized_value < 0.5 - center_margin) {
                normalized_value = this->scaleValue(normalized_value, 0., 0.5 - center_margin, 0., 0.5);
            }
        }
    }

    return (float)((int)(normalized_value * 1000.)) / 1000.f;
}

bool DataHandler::getCollectValues() { return this->collect_values; }

void DataHandler::setCollectValues(bool collect) { this->collect_values = collect; }

void DataHandler::clearPathValues()
{
    path_value_map_t::iterator iterator = this->path_values.begin();
    while (iterator != this->path_values.end()) {
        std::string  path              = iterator->first;
        path_value_t path_value        = iterator->second;
        this->path_values[path].value  = 0.;
        this->path_values[path].loaded = false;
        this->path_values[path].locked = false;
        iterator++;
    }
}

void DataHandler::setPathValue(std::string path, float value)
{
    if (this->path_values.find(path) != this->path_values.end()) {
        this->path_values[path].loaded = true;
        this->path_values[path].value  = value;
    }
}

Json::Value DataHandler::parseStringToJSON(std::string raw_json_string)
{
    JSONCPP_STRING err;
    Json::Value    root;
    const auto     raw_json_length = static_cast<int>(raw_json_string.length());

    Json::CharReaderBuilder                 builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(raw_json_string.c_str(), raw_json_string.c_str() + raw_json_length, &root, &err)) {
        throw "JSON parsing error.";
    }
    return root;
}

void DataHandler::printPathValues()
{
    path_value_map_t           pv_map      = this->path_values;
    path_value_map_t::iterator pv_iterator = pv_map.begin();

    std::cout << BACO_GRAY << "<-> Param States by Path:" << BACO_END;
    std::cout << BACO_GRAY << std::endl
              << this->rightPad("Path", 50) << this->leftPad("Value", 8) << this->leftPad("Loaded", 8) << this->leftPad("Locked", 8)
              << BACO_END << std::endl;
    while (pv_iterator != pv_map.end()) {
        std::string  path  = pv_iterator->first;
        path_value_t value = pv_iterator->second;

        std::cout << BACO_GRAY << this->rightPad(path, 50) << "" << std::setw(8) << value.value << "" << std::setw(8) << value.loaded
                  << "" << std::setw(8) << value.locked << BACO_END << std::endl;
        pv_iterator++;
    }
    std::cout << std::endl;
}

void DataHandler::printParamConfig(bool force_load)
{
    if (force_load) {
        this->loadConfig();
    }
    config_map_t config_map = this->param_config;

    std::cout << BACO_GRAY "\n\n<-> Loaded RainPots Config:" << std::endl;

    config_map_t::iterator unit_iterator = config_map.begin();
    while (unit_iterator != config_map.end()) {
        int unit = unit_iterator->first;
        std::cout << "\nUNIT: " << unit << std::endl;
        std::map<int, ctl_settings_t>::iterator ctl_iterator = unit_iterator->second.begin();
        while (ctl_iterator != unit_iterator->second.end()) {

            int ctl = ctl_iterator->first;

            ctl_settings_t settings = ctl_iterator->second;
            std::cout << "\t ctl: " << ctl << std::endl;
            std::cout << "\t   path: " << settings.path << std::endl;
            std::cout << "\t   steps: " << settings.steps << std::endl;
            std::cout << "\t   center: " << settings.center << std::endl << std::endl;

            ctl_iterator++;
        }
        unit_iterator++;
    }
    std::cout << BACO_END;

    preset_index_map_t presets = this->presets;

    std::cout << BACO_GRAY "\n\n<-> Patcher Prests:" << BACO_END << std::endl;
    preset_index_map_t::iterator preset_iterator = presets.begin();

    std::cout << BACO_GRAY << "" << std::setw(10) << "Name"
              << "" << std::setw(7) << "Index" << BACO_END << std::endl;

    while (preset_iterator != presets.end()) {
        std::cout << BACO_GRAY << "" << std::setw(10) << preset_iterator->first << "" << std::setw(7) << preset_iterator->second
                  << BACO_END << std::endl;
        preset_iterator++;
    }
    std::cout << BACO_END << std::endl;

    this->printPathValues();
}

char DataHandler::formatButtonValue(int unit, int button_index, float raw_value)
{
    char formatted_value = 0;

    int steps = (button_index == 1 || button_index == 2) ? this->getConrollerSteps(unit, button_index) : 2;
    steps     = (steps < 2) ? 2 : steps;
    steps     = (steps > 5) ? 5 : steps;

    formatted_value = (int)std::round((steps - 1.) * raw_value);

    return formatted_value;
}

/* protected methods*/

void DataHandler::loadConfig()
{
    std::map<int, std::map<int, ctl_settings_t>> config_map;
    std::string                                  rawJson;
    JSONCPP_STRING                               err;

    CURL*    curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "localhost");
        curl_easy_setopt(curl, CURLOPT_PORT, 5678);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DataHandler::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawJson);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    try {
        Json::Value root = this->parseStringToJSON(rawJson);

        // Parsing Parameters into map
        const Json::Value params = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["params"]["CONTENTS"];

        std::vector<std::string> param_names = params.getMemberNames();

        for (int i = 0; i < param_names.size(); i++) {
            // Member name and value
            Json::Value rainpot_config = params[param_names[i]]["CONTENTS"]["meta"]["CONTENTS"]["rainpots"];
            if (!rainpot_config.isNull()) {
                Json::Value unit            = rainpot_config["CONTENTS"]["unit"]["VALUE"];
                Json::Value ctl             = rainpot_config["CONTENTS"]["ctl"]["VALUE"];
                Json::Value center          = rainpot_config["CONTENTS"]["center"]["VALUE"];
                Json::Value steps           = rainpot_config["CONTENTS"]["steps"]["VALUE"];
                Json::Value normalized_path = params[param_names[i]]["CONTENTS"]["normalized"]["FULL_PATH"];

                if (!unit.isNull() && !ctl.isNull()) {
                    int         value_unit   = unit.asInt();
                    int         value_ctl    = ctl.asInt();
                    bool        value_center = (center.isNull()) ? false : center.asBool();
                    int         value_steps  = (steps.isNull()) ? -1 : steps.asInt();
                    std::string value_path   = normalized_path.asString();

                    if (config_map.find(value_unit) == config_map.end()) {
                        config_map.insert(std::make_pair(value_unit, std::map<int, ctl_settings_t>()));
                    }
                    ctl_settings_t ctl_settings;
                    ctl_settings.center = value_center;
                    ctl_settings.steps  = value_steps;
                    ctl_settings.path   = value_path;
                    config_map[value_unit].insert(std::make_pair(value_ctl, ctl_settings));
                    path_value_t path_value_entry;
                    this->path_values.insert(std::make_pair(value_path, path_value_entry));
                }
            }
        }
        this->param_config = config_map;

        // Parsing prests into map
        const Json::Value presets
            = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["presets"]["CONTENTS"]["entries"]["VALUE"];
        preset_index_map_t preset_index_map;
        for (int preset_index = 0; preset_index < presets.size(); preset_index++) {
            preset_index_map.insert(std::make_pair(presets[preset_index].asString(), preset_index));
        }
        this->presets = preset_index_map;
    } catch (...) {
        std::cerr << "JSON parsing error." << std::endl;
        return;
    }
}

float DataHandler::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
    float mapped = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    return this->clipValue(mapped, out_min, out_max);
}

float DataHandler::clipValue(float x, float min, float max)
{
    x = (x > max) ? max : x;
    x = (x < min) ? min : x;
    return x;
}

std::string DataHandler::rightPad(std::string const& str, int padding)
{
    std::ostringstream ss;
    ss << std::left << std::setfill(' ') << std::setw(padding) << str;
    return ss.str();
}

std::string DataHandler::leftPad(std::string const& str, int padding)
{
    std::ostringstream ss;
    ss << std::right << std::setfill(' ') << std::setw(padding) << str;
    return ss.str();
}