#include <jsoncpp/json/json.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <string>
#include <map>
#include <iterator>

typedef struct
{
    bool center = false;
    int steps = -1;
    std::string path = "";
} ctl_settings_t;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int main()
{
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rawJson);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    const auto rawJsonLength = static_cast<int>(rawJson.length());

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root, &err))
    {
        std::cout << "error" << std::endl;
        return EXIT_FAILURE;
    }
    const Json::Value params = root["CONTENTS"]["rnbo"]["CONTENTS"]["inst"]["CONTENTS"]["0"]["CONTENTS"]["params"]["CONTENTS"];

    std::vector<std::string> paramNames = params.getMemberNames();
    std::map<int, std::map<int, ctl_settings_t>> config_map;

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
            }
            else
            {
                std::cout << "NOT CONFIGURED PARAM: " << paramNames[i] << std::endl;
            }
        }
    }
    std::cout << "\n\nLOADED PAINPOTS CONFIG:" << std::endl;

    std::map<int, std::map<int, ctl_settings_t>>::iterator unit_iterator = config_map.begin();
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
            std::cout << "\t   center: " << settings.center << "\n" << std::endl;

            ctl_iterator++;
        }
        unit_iterator++;
    }
    return EXIT_SUCCESS;

    // CONTENTS rnbo CONTENTS inst CONTENTS 0 CONTENTS params CONTENTS
}
