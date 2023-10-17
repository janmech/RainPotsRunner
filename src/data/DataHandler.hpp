#ifndef __DATA_HANDLER__
#define __DATA_HANDLER__
#include "../osc/OscSender.hpp"
#ifdef __APPLE__
#include <json/json.h>
#else
#include <jsoncpp/json/json.h>
#endif
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

typedef struct
{
    bool loaded = false;
    float value = 0.;
} path_value_t;

typedef std::map<int, std::map<int, ctl_settings_t>> config_map_t;
typedef std::map<std::string, path_value_t> path_value_map_t;

class DataHandler
{
public:
    DataHandler()
    {
    }

    config_map_t getParams(bool force_load = false);
    bool getCollectValues();
    void setCollectValues(bool collect);
    void clearPathValues();
    void setPathValue(std::string path, float value);

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    void printParamConfig(bool force_load = false);
    void printPathValues();

protected:
    config_map_t param_config;
    path_value_map_t path_values;
    bool collect_values = false;
    void loadConfig();
};
#endif