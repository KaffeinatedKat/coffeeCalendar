#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>
#include <errno.h>

#include "ccal.h"

// Where to read events from
// TODO: download and convert ical files with libcurl
#define CCAL_LOCATION "/home/coffee/calendars/john.ccal"

// Event tag placeholders to use if they
// are not found within the ical event
#define PLACEHOLDER_EVENT_NAME "NoName"
#define PLACEHOLDER_EVENT_DATE "0000/00/00"
#define PLACEHOLDER_EVENT_START_TIME "00:00"
#define PLACEHOLDER_EVENT_END_TIME "00:00"

struct config_options {
    int screen_height;
    int screen_width;
    int refresh_time;
    long int current_day_bgcolor;
    char **online_calendars;
    char **calendar_colors;
};

int config_create(struct config_options *config, char *config_location);
#endif
