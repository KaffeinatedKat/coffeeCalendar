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

#include "calendar_utils.h"
#include "config.h"

struct file {
    char *content;
    size_t size;
};

struct event {
	char *name;
	char *cal_name;
	int all_day;
	struct tm start, end;
	struct tm date;
};

struct calendar {
	struct event *events;
	size_t nevents;
	size_t size;
};

int calendar_create(struct calendar *cal, char *file);
int calendar_destroy(struct calendar *cal);
int get_max_events_for_week(struct calendar *cal, int16_t year, int8_t month, int8_t day);
int get_number_of_events(struct calendar *cal, struct tm date);
#endif
