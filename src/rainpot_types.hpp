#ifndef __RAINPOT_TYPES__
#define __RAINPOT_TYPES__

#include <cstdarg>
#include <cstdint>
#include <map>
#include <string>

#define OSC_MESSAGE_TYPE_NONE        0
#define OSC_MESSAGE_TYPE_CC          1
#define OSC_MESSAGE_TYPE_PRESET_LOAD 2
#define OSC_MESSAGE_TYPE_PRESET_SAVE 3

#define PICK_UP_TURN_UP   0
#define PICK_UP_TURN_DOWN 1
#define PICK_UP_LOCKED    2

typedef struct queue_entry_message_t {
    uint8_t type        = OSC_MESSAGE_TYPE_NONE;
    char*   buffer      = NULL;
    int     buffer_size = 0;
} queue_entry_message_t;

typedef struct {
    bool        center = false;
    int         steps  = -1;
    std::string path   = "";
} ctl_settings_t;

typedef struct {
    bool  loaded = false;
    float value  = 0.;
    bool  locked = false;
} path_value_t;

typedef std::map<int, std::map<int, ctl_settings_t>> config_map_t;
typedef std::map<std::string, path_value_t>          path_value_map_t;
typedef std::map<std::string, int>                   preset_index_map_t;

typedef struct msg_osc_t {
    int         unit       = 0;
    int         controller = 0;
    std::string path       = "";
    std::string format     = "";
    std::string val_string = "";
    float       val_float  = 0.f;
} msg_osc_t;

typedef struct serial_queue_entry_t {
    char* buffer      = NULL;
    int   buffer_size = 0;

} serial_queue_entry_t;

typedef std::map<std::string, float> ctl_value_map_t;
#endif