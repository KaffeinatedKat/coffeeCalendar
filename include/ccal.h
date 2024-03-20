#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <regex.h>
#include <stdio.h>
#include <time.h>

#include "calendar_utils.h"
#include "config.h"

// Matches the input string with a regex string and outputs the match (only matches once)
char *regex_parse(char *input_string, char *regex_string);

// Return a char with all the events for any date
FILE *get_events(char *file, uint16_t year, uint8_t month, uint8_t day);

// Return the different properties of an event
char *get_event_name(char *event);
char *get_event_start_time(char *event);
char *get_event_end_time(char *event);
char *get_event_calendar_name(char *event);
bool get_event_all_day(char *event);

// Return the number of events for a given day
uint16_t get_number_of_events(FILE *ccal_event_list);

// Return the maximum number of events for any day in the given week number (0-52)
uint16_t get_max_events_for_week(uint16_t year, uint8_t month, uint8_t day);

