#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libical/ical.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "ccal.h"

void ical2ccal_load_events(struct ccal_calendar *cal, icalcomponent *ical_root, const char *calendar_name);
