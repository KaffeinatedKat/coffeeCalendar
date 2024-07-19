#pragma once

// ----- ical2ccal settings -----

// Set to 1 to enable online calendar syncing
#define SYNC_CALENDARS 1

//
// To set calendar colors, see line [x] in src/main.c
//

// Time to refresh the calendar in minutes (effects both on and offline)
#define CALENDAR_REFRESH_TIME 30

// Where to write ccal output files
#define CCAL_LOCATION "/home/coffee/calendars/john.ccal"

// Event tag placeholders to use if they
// are not found within the ical event
#define PLACEHOLDER_EVENT_NAME "NoName"
#define PLACEHOLDER_EVENT_DATE "0000/00/00"
#define PLACEHOLDER_EVENT_START_TIME "00:00"
#define PLACEHOLDER_EVENT_END_TIME "00:00"

// ----- coffeeCalendar settings -----

// The number of weeks to display (including current)
#define NUMBER_OF_WEEKS 5

#define TILE_BG_COLOR
#define CURRENT_DAY_BG_COLOR 0xADD8E6

#define SCREEN_HEIGHT 1080
#define SCREEN_WIDTH 1920
