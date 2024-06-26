#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <stdbool.h>

#include "config.h"
#include "calendar_utils.h"
#include "ccal.h"
#include "hashmap.h"


struct calendars {
    char *name;
    uint32_t color;
};


enum current_position {
    LAST_MONTH,
    THIS_MONTH,
    NEXT_MONTH,
    NEXT_NEXT_MONTH,
};

// <----- Hashmap.c stuff ----->
int color_compare(const void *a, const void *b, void *udata) {
    const struct calendars *ua = a;
    const struct calendars *ub = b;
    return strcmp(ua->name, ub->name);
}

uint64_t color_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct calendars *calendars = item;
    return hashmap_sip(calendars->name, strlen(calendars->name), seed0, seed1);
}
// <---------->

void render_calendar(lv_obj_t *cont) {
    // time 
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int16_t current_year = tm.tm_year + 1900;
    int8_t current_month = tm.tm_mon + 1;
    int8_t current_day = tm.tm_mday;
    int8_t start_day = day_of_week(current_year, current_month, current_day);
    int8_t end_day = last_day_of_month(current_month, is_leap_year(current_year));

    // calendar tiles
    int8_t current_tile_number = get_week_number(current_year, current_month, current_day);
    int8_t current_position = THIS_MONTH;
    int8_t this_month_day = 1;
    int8_t next_month_day = 1;
    int8_t modified_month = current_month;
    int16_t modified_year = current_year;
    int8_t first_tile_day;
    int8_t week_start_tile_number;
    bool month_bleed = false;

    // lvgl stuff
    const int16_t TILE_H = (SCREEN_HEIGHT / NUMBER_OF_WEEKS) - 8;
    const int16_t TILE_W = (SCREEN_WIDTH / 7);
    int16_t tile_x = TILE_W * -1;
    int16_t tile_y = TILE_H * -1;
    int16_t modified_tile_h = TILE_H;
    int16_t modified_tile_w = TILE_W;

    FILE *event_list;

    struct hashmap *map = hashmap_new(sizeof(struct calendars), 0, 0, 0, color_hash, color_compare, NULL, NULL);

    // Calendar names & colors
    // (program will segfault if your calendar name is not placed in the hashmap)
    hashmap_set(map, &(struct calendars){ .name = "mom.ics", .color = 0x023E8A });
    hashmap_set(map, &(struct calendars){ .name = "john_work.ics", .color = 0x276221 });
    hashmap_set(map, &(struct calendars){ .name = "keith_work.ics", .color = 0x276221 });
    hashmap_set(map, &(struct calendars){ .name = "family.ics", .color = 0xff781f });

    lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_pad_all(cont, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_radius(cont, LV_STATE_DEFAULT, 0);
    lv_obj_center(cont);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    // If the days in the current week go into last month, calculate
    // what the last day of that month was and count backwards from there
    if (current_day - start_day <= 0) {
        current_position = LAST_MONTH;
        month_bleed = true;
        first_tile_day = last_day_of_month(current_month - 1, is_leap_year(current_year)) + (current_day - start_day);
    // Otherwise we can just count back x number of times
    } else {
        first_tile_day = current_day - start_day;
    }

    // Actually rendering the calendar
    for (int r = 0; r < NUMBER_OF_WEEKS; r++) {
        int16_t max_events_this_week = 0;
        tile_x = TILE_W * -1;
        tile_y += TILE_H;

        max_events_this_week = get_max_events_for_week(current_year, modified_month, current_tile_number);

        // Increase the size of the tiles to fit all the events
        if (max_events_this_week > 3 && r != 0) {
            modified_tile_h += 50 * (max_events_this_week - 3);
            tile_y += 50 * (max_events_this_week - 3);
        } else {
            modified_tile_h += TILE_H;
        }

        if (r == 0) {
            tile_y += 40;
        }

        for (int c = 0; c < 7; c++) {
            const int8_t box_number = r * 7 + c + 1;
            int16_t event_pos = -25;
            char result[1024];

            modified_year = current_year;
            modified_month = current_month;
            tile_x += TILE_W;

            if (r == 0) {
                lv_obj_t *tile = lv_obj_create(cont);
                lv_obj_set_size(tile, TILE_W, 40);
                lv_obj_set_pos(tile, tile_x - 1, 0);
                lv_obj_set_style_radius(tile, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_pad_all(tile, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_bg_color(tile, lv_color_hex(0x282828), 0);

                lv_obj_t *week_label = lv_label_create(tile);
                lv_label_set_text_fmt(week_label, "%s", week_name(c));
                lv_obj_set_style_text_color(week_label, lv_color_hex(0xffffff), 0);
                lv_obj_set_align(week_label, LV_ALIGN_CENTER);
            }

            lv_obj_t * tile = lv_obj_create(cont);
            lv_obj_set_size(tile, modified_tile_w, modified_tile_h);
            lv_obj_set_pos(tile, tile_x - 1, tile_y);
            lv_obj_set_style_pad_all(tile, 20, 0);
            lv_obj_set_style_radius(tile, LV_STATE_DEFAULT, 0);
            lv_obj_set_style_border_width(tile, LV_STATE_DEFAULT, 1);
            lv_obj_set_style_border_color(tile, lv_color_hex(0xd9d9d9), 0);
            lv_obj_set_style_bg_color(tile, lv_color_hex(0x282828), 0);
            lv_obj_set_scrollbar_mode(tile, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t * day_label = lv_label_create(tile);
            lv_obj_set_size(day_label, 100, 54);
            lv_obj_set_align(day_label, LV_ALIGN_TOP_LEFT);
            lv_obj_set_style_text_color(day_label, lv_color_hex(0xd3d3d3), 0);

            lv_obj_t * month_label = lv_label_create(tile);
            lv_obj_set_size(month_label, 40, 54);
            lv_obj_set_pos(month_label, 200, 0);
            lv_obj_set_style_text_color(month_label, lv_color_hex(0xd3d3d3), 0);

            // Highlight current day in blue
            if (box_number == start_day + 1) {
                // Make the day and month labels visible with the different background color
                lv_obj_set_style_text_color(day_label, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_text_color(month_label, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_bg_color(tile, lv_color_hex(CURRENT_DAY_BG_COLOR), 0);
            }

            // Count the tiles up
            current_tile_number = first_tile_day++;

            if (current_position == LAST_MONTH) {
                // Adjust for year changes
                if (current_month == 1) {
                    modified_month = 12;
                    modified_year = current_year - 1;
                } else {
                    modified_month = current_month - 1;
                }

                if (current_tile_number > last_day_of_month(current_month - 1, is_leap_year(current_year)) - 1) {
                    first_tile_day = this_month_day++;
                    current_position = THIS_MONTH;
                }

            } else if (current_position == THIS_MONTH) {
                // Adjust for year changes
                if (current_month == 12) {
                    modified_month = 1;
                    modified_year = current_year + 1;
                } else {
                    modified_month = current_month;
                }

                if (current_tile_number > last_day_of_month(current_month - 1, is_leap_year(current_year)) - 1) {
                    first_tile_day = next_month_day++;
                    current_position = NEXT_MONTH;
                }

            } else if (current_position == NEXT_MONTH) {
                modified_month = current_month + 1;
                if (current_tile_number > last_day_of_month(modified_month, is_leap_year(current_year)) - 1) {
                    first_tile_day = 1;
                    current_position = NEXT_NEXT_MONTH;
                }
            } else if (current_position == NEXT_NEXT_MONTH) {
                modified_month = current_month + 2;
            }

            event_list = get_events(CCAL_LOCATION, modified_year, modified_month, current_tile_number);

            // Loop through each event and add it to the day box
            while (fgets(result, sizeof(result), event_list)) {
                struct calendars *calendars;
                bool event_all_day = get_event_all_day(result);
                char *event_name = get_event_name(result);
                char *event_calendar_name = get_event_calendar_name(result);

                calendars = hashmap_get(map, &(struct calendars){ .name=event_calendar_name });
                uint32_t event_color = calendars->color; 

                // Event label
                lv_obj_t *event_container = lv_obj_create(tile);
                lv_obj_set_size(event_container, 225, 43);
                lv_obj_set_style_pad_left(event_container, LV_STATE_DEFAULT, 5);
                lv_obj_set_style_bg_color(event_container, lv_color_hex(event_color), 0);
                lv_obj_set_style_border_width(event_container, LV_STATE_DEFAULT, 0);
                lv_obj_set_scrollbar_mode(event_container, LV_SCROLLBAR_MODE_OFF);
                event_pos += 50;

                lv_obj_t * event_label = lv_label_create(event_container);
                lv_obj_set_style_text_color(event_label, lv_color_hex(0xffffff), 0);

                if (!event_all_day) {
                    char *event_start_time = get_event_start_time(result);
                    char *event_end_time = get_event_end_time(result);

                    lv_label_set_text_fmt(event_label, "%s\n%s - %s", event_name, event_start_time, event_end_time);

                    free(event_start_time);
                    free(event_end_time);
                } else {
                    lv_label_set_text_fmt(event_label, "%s\nAll day", event_name);
                }

                lv_obj_set_pos(event_container, 0, event_pos);
                lv_obj_set_align(event_label, LV_ALIGN_LEFT_MID);

                free(event_name);
                free(event_calendar_name);
            }
            pclose(event_list);

            lv_label_set_text_fmt(day_label, "%d", current_tile_number);
            lv_label_set_text_fmt(month_label, "%s", month_name(modified_month));
        }
    }
    lv_obj_set_style_pad_row(cont, 0, 0);
    lv_obj_set_style_pad_column(cont, 0, 0);
}

int main(int argc, char *argv[])
{
    lv_init();

    /*Linux frame buffer device init*/
    lv_display_t * disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    // Create the screen
    lv_obj_t* cont = lv_obj_create(lv_screen_active());

    // Set up the hashmap for calendar colors

    render_calendar(cont);

    /*Handle LVGL tasks*/
    while(1) {
        lv_timer_handler();

        sleep(CALENDAR_REFRESH_TIME * 60);

        // Sync the new online calendar data (if applicible)
        // use cron to download new ical files and convert them with bin/ical2ccal
        // FIXME: include builtin online calendar refreshing

        // Clean up the screen and redraw it
        lv_obj_clean(cont);
        render_calendar(cont);
    }

    return 0;
}

