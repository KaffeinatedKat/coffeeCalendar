#ifndef CCAL
#define CCAL
#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "config.h"

struct file {
    char *content;
    size_t size;
};

struct ccal_event {
	const char *name;
	const char *cal_name;
	int all_day;
    int color_index;
	struct tm start, end;
	struct tm date;
};

struct ccal_calendar {
	struct ccal_event *events;
	size_t nevents;
	size_t size;
};

int ccal_read_file(struct file *event_list, const char *ccal_file);
int ccal_add_event(struct ccal_calendar *cal, struct ccal_event event);
int ccal_calendar_create(struct ccal_calendar *cal, char *file);
int ccal_calendar_destroy(struct ccal_calendar *cal);
int ccal_get_max_events_for_week(struct ccal_calendar *cal, struct tm date);
int ccal_get_number_of_events(struct ccal_calendar *cal, struct tm date);

#endif

