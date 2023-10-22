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

bool DataHandler::contollerIsAssigned(int unit, int controller)
{
    return (this->param_config.find(unit) != this->param_config.end()) && (this->param_config[unit].find(controller) != this->param_config[unit].end());
}

bool DataHandler::controllerIsCentered(int unit, int controller)
{
    bool is_centered = false;
    if (this->contollerIsAssigned(unit, controller)) {
        is_centered = this->param_config[unit][controller].center;
    }
    return is_centered;
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

    std::cout << std::endl << BACO_GRAY << "<-> Param Values by Path:" << BACO_END << std::endl;
    std::cout << BACO_GRAY << std::endl << this->rightPad("Path", 50) << this->leftPad("Value", 8) << this->leftPad("Loaded", 8) << this->leftPad("Locked", 8) << BACO_END << std::endl;
    while (pv_iterator != pv_map.end()) {
        std::string  path  = pv_iterator->first;
        path_value_t value = pv_iterator->second;

        std::cout << BACO_GRAY << this->rightPad(path, 50) << "" << std::setw(8) << value.value << "" << std::setw(8) << value.loaded << "" << std::setw(8) << value.locked << BACO_END << std::endl;
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
            int            ctl      = ctl_iterator->first;
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
        std::cout << BACO_GRAY << "" << std::setw(10) << preset_iterator->first << "" << std::setw(7) << preset_iterator->second << BACO_END << std::endl;
        preset_iterator++;
    }
    std::cout << BACO_END;

    this->printPathValues();
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
        const Json::Value  presets = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["presets"]["CONTENTS"]["entries"]["VALUE"];
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