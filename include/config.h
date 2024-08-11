#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#include "error.h"
#include "ccal.h"

struct config_options {
    int screen_height;
    int screen_width;
    int refresh_time;
    int calendar_count;
    int event_blacklist_size;
    int log_level;
    long int current_day_bgcolor;
    char config_path[2048];
    char filecache_path[2048];
    char **online_calendars;
    char **calendar_colors;
    char **event_blacklist;
};

int config_create(struct config_options *config, char *config_location);

#endif
