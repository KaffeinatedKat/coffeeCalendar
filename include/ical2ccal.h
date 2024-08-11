#ifndef ICAL2CCAL
#define ICAL2CCAL
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libical/ical.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "ccal.h"

enum ic2cc_errs {
    IC2CC_NO_ERR,
    IC2CC_NO_EVENT_NAME,
    IC2CC_NO_START_INFO,
    IC2CC_NO_END_INFO,
};

int ical2ccal_load_events(struct ccal_calendar *cal, icalcomponent *ical_root, char *calendar_name, char **blacklist, int blacklist_size, int log_level);

#endif
