#include "../lvgl/lvgl.h"
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

void render_calendar(lv_obj_t *cont, struct config_options *config) {
    // time 
    time_t t = time(NULL);
    struct tm date = *localtime(&t);
    int16_t current_year = date.tm_year + 1900;
    int8_t current_month = date.tm_mon + 1;
    int8_t current_day = date.tm_mday;
    int8_t current_tile_number;
    int8_t start_day = day_of_week(current_year, current_month, current_day);

    // lvgl stuff
    const int16_t TILE_H = (config->screen_height / 5) - 14;
    const int16_t TILE_W = (config->screen_width / 7);
    int16_t tile_x = TILE_W * -1;
    int16_t tile_y = TILE_H * -1;
    int16_t modified_tile_h = TILE_H;
    int16_t modified_tile_w = TILE_W;

    // Calendar events
    FILE *event_list;
    struct calendar cal = {0};
    struct event *event;

    // Misc
    char *endptr;


    calendar_create(&cal, CCAL_LOCATION);

    struct hashmap *map = hashmap_new(sizeof(struct calendars), 0, 0, 0, color_hash, color_compare, NULL, NULL);

    // Calendar names & colors
    // (program will segfault if your calendar name is not placed in the hashmap)
    hashmap_set(map, &(struct calendars){ .name = "mom.ics", .color = strtol("0x023E8A", &endptr, 16) });
    hashmap_set(map, &(struct calendars){ .name = "john_work.ics", .color = 0x276221 });
    hashmap_set(map, &(struct calendars){ .name = "keith_work.ics", .color = 0x276221 });
    hashmap_set(map, &(struct calendars){ .name = "family.ics", .color = 0xff781f });

    // Configure the screen
    lv_obj_set_size(cont, config->screen_width, config->screen_height);
    lv_obj_set_style_pad_all(cont, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_radius(cont, LV_STATE_DEFAULT, 0);
    lv_obj_center(cont);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    // Get the first tile's date
    date.tm_mday = current_day - start_day - 1;
    mktime(&date);

    // Actually rendering the calendar
    for (int r = 0; r < 5; r++) {
        int16_t max_events_this_week = 0;
        tile_x = TILE_W * -1;
        tile_y += TILE_H;

        max_events_this_week = get_max_events_for_week(&cal, current_year, current_month, current_tile_number);

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

            tile_x += TILE_W;

            if (r == 0) {
                // Top bar with days of the week
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
                lv_obj_set_style_bg_color(tile, lv_color_hex(config->current_day_bgcolor), 0);
            }

            // Count the tiles up
            date.tm_mday += 1;
            // Update the time struct
            mktime(&date);

            current_tile_number = date.tm_mday;
            current_month = date.tm_mon + 1;
            current_year = date.tm_year + 1900;

            // Loop through each event and add it to the day box
            for (int x = 0; x < cal.nevents; x++) {
                event = &cal.events[x];

                // Only grab events for the current date
                if (event->date.tm_year + 1900 != current_year) continue;
                else if (event->date.tm_year + 1900 > current_year) break;
                if (event->date.tm_mon + 1 != current_month) continue;
                else if (event->date.tm_mon + 1 > current_month) break;
                if (event->date.tm_mday != current_tile_number) continue;
                else if (event->date.tm_mday > current_tile_number) break;

                struct calendars *calendars;
                bool event_all_day = event->all_day;
                const char *event_name = event->name;
                const char *event_calendar_name = event->cal_name;

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
                    char event_start_time[6] = {0};
                    char event_end_time[6] = {0};

                    strftime(event_start_time, 6, "%H:%M", &event->start);
                    strftime(event_end_time, 6, "%H:%M", &event->end);

                    lv_label_set_text_fmt(event_label, "%s\n%s - %s", event_name, event_start_time, event_end_time);
                } else {
                    lv_label_set_text_fmt(event_label, "%s\nAll day", event_name);
                }

                lv_obj_set_pos(event_container, 0, event_pos);
                lv_obj_set_align(event_label, LV_ALIGN_LEFT_MID);
            }

            lv_label_set_text_fmt(day_label, "%d", current_tile_number);
            lv_label_set_text_fmt(month_label, "%s", month_name(current_month));
        }
    }
    lv_obj_set_style_pad_row(cont, 0, 0);
    lv_obj_set_style_pad_column(cont, 0, 0);

    // Sync info bar at the bottom
    lv_obj_t * info_bar = lv_obj_create(cont);
    lv_obj_set_size(info_bar, config->screen_width, 30);
    lv_obj_set_pos(info_bar, 0, config->screen_height - 30);
    lv_obj_set_style_pad_all(info_bar, 20, 0);
    lv_obj_set_style_radius(info_bar, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_border_width(info_bar, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_border_color(info_bar, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_color(info_bar, lv_color_hex(0x282828), 0);
    lv_obj_set_scrollbar_mode(info_bar, LV_SCROLLBAR_MODE_OFF);

    /*
    TODO: add last sync time to info bar
    lv_obj_t *sync_label = lv_label_create(info_bar);
    lv_label_set_text_fmt(sync_label, "Last synced: 21:30");
    lv_obj_set_style_text_color(sync_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(sync_label, LV_ALIGN_CENTER);
    */

    lv_obj_t *version_label = lv_label_create(info_bar);
    lv_label_set_text_fmt(version_label, "coffeeCalendar Beta v1.1.0");
    lv_obj_set_style_text_color(version_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(version_label, LV_ALIGN_LEFT_MID);

    lv_obj_t *url_label = lv_label_create(info_bar);
    lv_label_set_text_fmt(url_label, "https://github.com/KaffeinatedKat/coffeeCalendar");
    lv_obj_set_style_text_color(url_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(url_label, LV_ALIGN_RIGHT_MID);

    calendar_destroy(&cal);
}

int main(int argc, char *argv[])
{
    const char *home = getenv("HOME");
    char config_path[2048];
    strcpy(config_path, home);
    strcat(config_path, "/.config/coffeeCalendar/config");
    
    // Parse the config file
    struct config_options config = {0};
    config_create(&config, config_path);

    lv_init();

    /*Linux frame buffer device init*/
    lv_display_t * disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    // Create the screen
    lv_obj_t* cont = lv_obj_create(lv_screen_active());

    do {
        // Render the lvgl screen
        render_calendar(cont, &config);

        lv_timer_handler();

        sleep(config.refresh_time * 60);

        // Sync the new online calendar data (if applicible)
        // use cron to download new ical files and convert them with bin/ical2ccal
        // FIXME: include builtin online calendar refreshing
    

        // Clean up the screen and redraw it
        lv_obj_clean(cont);
        
    } while (1);

    lv_obj_clean(cont);

    return 0;
}

