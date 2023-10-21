#include "DataHandler.hpp"

config_map_t DataHandler::getParams(bool force_load)
{
    if (this->param_config.size() == 0 || force_load)
    {
        try
        {
            this->loadConfig();
        }
        catch (const char *msg)
        {
            std::cerr << msg << std::endl;
        }
    }
    return this->param_config;
}

std::string DataHandler::getPathForController(int unit, int controller)
{
    std::string osc_path = "";
    if (this->param_config.find(unit) != this->param_config.end())
    {
        if (this->param_config[unit].find(controller) != this->param_config[unit].end())
        {
            osc_path = this->param_config[unit][controller].path;
        }
    }
    return osc_path;
}

bool DataHandler::getCollectValues()
{
    return this->collect_values;
}

void DataHandler::setCollectValues(bool collect)
{
    this->collect_values = collect;
}

void DataHandler::clearPathValues()
{
    path_value_map_t::iterator iterator = this->path_values.begin();
    while (iterator != this->path_values.end())
    {
        std::string path = iterator->first;
        path_value_t path_value = iterator->second;
        this->path_values[path].loaded = false;
        this->path_values[path].value = 0.;
        iterator++;
    }
}

void DataHandler::setPathValue(std::string path, float value)
{
    if (this->path_values.find(path) != this->path_values.end())
    {
        this->path_values[path].loaded = true;
        this->path_values[path].value = value;
    }
}

void DataHandler::printPathValues()
{
    path_value_map_t::iterator iterator = this->path_values.begin();
    while (iterator != this->path_values.end())
    {
        std::cout << iterator->first << "\n  loaded: " << iterator->second.loaded << "\n  value: " << iterator->second.value << std::endl;
        iterator++;
    }
}

void DataHandler::printParamConfig(bool force_load)
{
    if (force_load)
    {
        this->loadConfig();
    }
    config_map_t config_map = this->param_config;

    std::cout << "\n\nLOADED PAINPOTS CONFIG:" << std::endl;

    config_map_t::iterator unit_iterator = config_map.begin();
    while (unit_iterator != config_map.end())
    {
        int unit = unit_iterator->first;
        std::cout << "\nUNIT: " << unit << std::endl;
        std::map<int, ctl_settings_t>::iterator ctl_iterator = unit_iterator->second.begin();
        while (ctl_iterator != unit_iterator->second.end())
        {
            int ctl = ctl_iterator->first;
            ctl_settings_t settings = ctl_iterator->second;
            std::cout << "\t ctl: " << ctl << std::endl;
            std::cout << "\t   path: " << settings.path << std::endl;
            std::cout << "\t   steps: " << settings.steps << std::endl;
            std::cout << "\t   center: " << settings.center << "\n"
                      << std::endl;

            ctl_iterator++;
        }
        unit_iterator++;
    }
}

/* protected methods*/
void DataHandler::loadConfig()
{

    std::map<int, std::map<int, ctl_settings_t>> config_map;
    std::string rawJson;
    JSONCPP_STRING err;
    Json::Value root;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "localhost");
        curl_easy_setopt(curl, CURLOPT_PORT, 5678);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DataHandler::writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawJson);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    const auto rawJsonLength = static_cast<int>(rawJson.length());

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root, &err))
    {
        throw "JSON parsing errer.";
    }
    const Json::Value params = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["params"]["CONTENTS"];

    std::vector<std::string> paramNames = params.getMemberNames();

    for (int i = 0; i < paramNames.size(); i++)
    {
        // Member name and value
        Json::Value rainpot_config = params[paramNames[i]]["CONTENTS"]["meta"]["CONTENTS"]["rainpots"];
        if (!rainpot_config.isNull())
        {
            Json::Value unit = rainpot_config["CONTENTS"]["unit"]["VALUE"];
            Json::Value ctl = rainpot_config["CONTENTS"]["ctl"]["VALUE"];
            Json::Value center = rainpot_config["CONTENTS"]["center"]["VALUE"];
            Json::Value steps = rainpot_config["CONTENTS"]["steps"]["VALUE"];
            Json::Value normalized_path = params[paramNames[i]]["CONTENTS"]["normalized"]["FULL_PATH"];

            if (!unit.isNull() && !ctl.isNull())
            {
                int value_unit = unit.asInt();
                int value_ctl = ctl.asInt();
                bool value_center = (center.isNull()) ? false : center.asBool();
                int value_steps = (steps.isNull()) ? -1 : steps.asInt();
                std::string value_path = normalized_path.asString();

                if (config_map.find(value_unit) == config_map.end())
                {
                    config_map.insert(std::make_pair(value_unit, std::map<int, ctl_settings_t>()));
                }
                ctl_settings_t ctl_settings;
                ctl_settings.center = value_center;
                ctl_settings.steps = value_steps;
                ctl_settings.path = value_path;
                config_map[value_unit].insert(std::make_pair(value_ctl, ctl_settings));
                path_value_t path_value_entry;
                this->path_values.insert(std::make_pair(value_path, path_value_entry));
            }
        }
    }
    this->param_config = config_map;
}