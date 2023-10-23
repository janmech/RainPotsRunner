#ifndef __DATA_HANDLER__
#define __DATA_HANDLER__
#ifdef __APPLE__
#include <json/json.h>
#else
#include <jsoncpp/json/json.h>
#endif
#include <curl/curl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include "../bash_colors.hpp"
#include "../rainpot_types.hpp"

class DataHandler {
public:
    DataHandler(bool debug = false) { this->debug = debug; }

    config_map_t       getParams(bool force_load = false);
    preset_index_map_t getPresets(bool force_load = false);
    bool               getCollectValues();
    void               setCollectValues(bool collect);
    void               clearPathValues();
    void               setPathValue(std::string path, float value);
    bool               contollerIsAssigned(int rainpot_uni, int controller);
    bool               controllerIsCentered(int unit, int controller);
    float              makeValueFLoat(int unit, int controler, int raw_value);
    std::string        getPathForController(int unit, int controller);
    Json::Value        parseStringToJSON(std::string raw_json_string);
    // bool                makeValuePickupMessasge(queue_entry_message_t* msg,serial_queue_entry_t* serial_queue_entry);

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void printParamConfig(bool force_load = false);
    void printPathValues();

protected:
    bool               debug = false;
    config_map_t       param_config;
    path_value_map_t   path_values;
    preset_index_map_t presets; // Wecurrently don't use this map. It's still here for future features
    bool               collect_values = false;
    void               loadConfig();
    float              scaleValue(float x, float in_min = 0., float in_max = 1., float out_min = 0., float out_max = 0.);
    float              clipValue(float x, float min = 0., float max = 1.);
    std::string        rightPad(std::string const& str, int padding);
    std::string        leftPad(std::string const& str, int padding);
};
#endif