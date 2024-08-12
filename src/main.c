#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <stdbool.h>

#include "config.h"
#include "calutils.h"
#include "ical2ccal.h"
#include "ical.h"
#include "ccal.h"


struct info_bar_data {
    time_t last_synced;
    time_t current_time;
};

void render_info_bar(struct info_bar_data *data, lv_obj_t *cont, struct config_options *config) {
    struct tm *time;
    char sync_text[24] = {0};
    int sync_time = 0;

    lv_obj_t * info_bar = lv_obj_create(cont);
    lv_obj_set_size(info_bar, config->screen_width, 30);
    lv_obj_set_pos(info_bar, 0, config->screen_height - 30);
    lv_obj_set_style_pad_all(info_bar, 20, 0);
    lv_obj_set_style_radius(info_bar, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_border_width(info_bar, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_border_color(info_bar, lv_color_hex(0xd9d9d9), 0);
    lv_obj_set_style_bg_color(info_bar, lv_color_hex(0x282828), 0);
    lv_obj_set_scrollbar_mode(info_bar, LV_SCROLLBAR_MODE_OFF);

    sync_time = difftime(data->current_time, data->last_synced) / 60;
    if (sync_time == 0) {
        snprintf(sync_text, 22, "Last synced just now");
    } else {
        snprintf(sync_text, 20, "Last synced %dm ago", sync_time);
    }
    lv_obj_t *sync_label = lv_label_create(info_bar);
    lv_label_set_text(sync_label, sync_text);
    lv_obj_set_style_text_color(sync_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(sync_label, LV_ALIGN_CENTER);

    lv_obj_t *version_label = lv_label_create(info_bar);
    lv_label_set_text(version_label, "coffeeCalendar Beta v1.2.4");
    lv_obj_set_style_text_color(version_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(version_label, LV_ALIGN_LEFT_MID);

    time = localtime(&data->current_time);
    lv_obj_t *time_label = lv_label_create(info_bar);
    lv_label_set_text_fmt(time_label, "[ %02d:%02d ]", time->tm_hour, time->tm_min);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_align(time_label, LV_ALIGN_RIGHT_MID);
}

void render_calendar(struct ccal_calendar cal, lv_obj_t *cont, struct config_options *config) {
    // time 
    time_t t = time(NULL);
    struct tm date = *localtime(&t);
    int16_t current_year = date.tm_year + 1900;
    int8_t current_month = date.tm_mon + 1;
    int8_t current_day = date.tm_mday;
    int8_t current_tile_number;
    int8_t start_day = calutils_first_day_of_week(current_year, current_month, current_day);

    // lvgl stuff
    const int16_t TILE_H = (config->screen_height / 5) - 14;
    const int16_t TILE_W = (config->screen_width / 7);
    int16_t tile_x = TILE_W;
    int16_t tile_y = TILE_H;
    int16_t modified_tile_h = TILE_H;
    int16_t modified_tile_w = TILE_W;

    char *endptr;

    // Calendar events
    FILE *event_list;
    struct ccal_event *event;

    // Configure the screen
    lv_obj_set_size(cont, config->screen_width, config->screen_height);
    lv_obj_set_style_pad_all(cont, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_radius(cont, LV_STATE_DEFAULT, 0);
    lv_obj_center(cont);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    // Get the first tile's date
    date.tm_mday = current_day - start_day - 1;
    current_tile_number = date.tm_mday;
    current_month = date.tm_mon + 1;
    current_year = date.tm_year + 1900;
    mktime(&date);

    int add_y = 0;
    tile_y = 0;

    // Actually rendering the calendar
    for (int r = 0; r < 5; r++) {
        int16_t max_events_this_week = 0;

        tile_x = TILE_W * -1;
        add_y = TILE_H;
        max_events_this_week = ccal_get_max_events_for_week(&cal, date);

        if (r == 0) {
            tile_y += 40;
        }

        // Increase the size of the tiles to fit all the events
        if (max_events_this_week > 3) {
            modified_tile_h += 50 * (max_events_this_week - 3);
            add_y += 50 * (max_events_this_week - 3);
        } else {
            modified_tile_h += TILE_H;
        }

        for (int c = 0; c < 7; c++) {
            const int8_t box_number = r * 7 + c + 1;
            int16_t event_pos = -25;
            char result[1024];

            tile_x += TILE_W;

            // Top bar with days of the week
            if (r == 0) {
                lv_obj_t *tile = lv_obj_create(cont);
                lv_obj_set_size(tile, TILE_W, 40);
                lv_obj_set_pos(tile, tile_x - 1, 0);
                lv_obj_set_style_radius(tile, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_pad_all(tile, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_bg_color(tile, lv_color_hex(0x282828), 0);

                lv_obj_t *week_label = lv_label_create(tile);
                lv_label_set_text_fmt(week_label, "%s", calutils_week_name(c));
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
                if (config->log_level > 1) {
                    printf("%s", ANSI_YELLOW);
                }
            }

            // Count the tiles up
            date.tm_mday += 1;
            // Update the time struct
            mktime(&date);

            current_tile_number = date.tm_mday;
            current_month = date.tm_mon + 1;
            current_year = date.tm_year + 1900;

            if (config->log_level > 1) {
                printf("%02d %s", current_tile_number, ANSI_RESET);
            }

            // Loop through each event and add it to the day box
            for (int x = 0; x < cal.nevents; x++) {
                event = &cal.events[x];

                // Only grab events for the current date
                if (event->date.tm_year + 1900 != current_year) continue;
                if (event->date.tm_mon + 1 != current_month) continue;
                if (event->date.tm_mday != current_tile_number) continue;

                bool event_all_day = event->all_day;
                const char *event_name = event->name;
                const char *event_calendar_name = event->cal_name;

                uint32_t event_color = strtol(config->calendar_colors[event->color_index], &endptr, 16);

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
            lv_label_set_text_fmt(month_label, "%s", calutils_month_name(current_month));
        }

        if (config->log_level > 1) {
            printf("\n");
        }

        tile_y += add_y;
    }
    lv_obj_set_style_pad_row(cont, 0, 0);
    lv_obj_set_style_pad_column(cont, 0, 0);

}

int main(int argc, char *argv[])
{
    static const struct ccal_calendar empty_cal = {NULL, 0, 0};
    struct ccal_calendar cal = {NULL, 0, 0};
    struct info_bar_data data = {0};
    const char *home = getenv("HOME");
    char ical_file_path[4096];
    char ical_name[9];
    int x = 0;
    int err = 0;
    time_t rawtime;

    printf("coffeeCalendar Beta v1.2.4\n\n");

    // Load file storage path into config
    struct config_options config = {0};
    struct stat st = {0};
    strcpy(config.config_path, home);
    strcat(config.config_path, "/.config/coffeeCalendar/config");
    strcpy(config.filecache_path, home);
    strcat(config.filecache_path, "/.local/share/coffeeCalendar/");
    // Create the ical storage directory if it doesn't already exist
    if (stat(config.filecache_path, &st) == -1) {
        err = mkdir(config.filecache_path, 0700);
        if (err == -1) {
            error_log("Failed to create '~/.local/share/coffeeCalendar'\nDoes '~/.local/share` exist?\n", E_ERROR);
            exit(-1);
        }
    }

    // Parse the config file
    config_create(&config, config.config_path);
    // Refresh the calendars immediately on startup
    x = config.refresh_time;
    if (config.log_level > 0) {
        printf("Downloading ical files...\n");
    }

    lv_init();

    time(&rawtime);
    data.current_time = mktime(localtime(&rawtime));
    data.last_synced = mktime(localtime(&rawtime));

    /*Linux frame buffer device init*/
    lv_display_t * disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    // Create the screen
    lv_obj_t* cont = lv_obj_create(lv_screen_active());

    do {
        // Loop every minute for info bar, refresh calendars every x == config.refresh_time
        x++;
        if (x >= config.refresh_time) {
            // Clear old events
            ccal_calendar_destroy(&cal);
            cal = empty_cal;

            // Online calendar downloading
            // Download online calendars with curl
            for (int x = 0; x < config.calendar_count; x++) {
                ical_name[0] = '\0';
                snprintf(ical_name, 9, "%03d.ical", x);
                ical_file_path[0] = '\0';
                strcpy(ical_file_path, config.filecache_path);
                strcat(ical_file_path, ical_name);

                ical_download(config.online_calendars[x], ical_file_path);
                struct file ical_data;
                ccal_read_file(&ical_data, ical_file_path);
                icalcomponent *ical_root = icalparser_parse_string(ical_data.content);
                //if (ical_root == NULL) { printf("Failed to parse ical data\n"); }

                ical2ccal_load_events(&cal, ical_root, ical_file_path, config.event_blacklist, config.event_blacklist_size, config.log_level);
            }
            time(&data.last_synced);

            // Rerender the calendar with new events
            lv_obj_clean(cont);

            if (config.log_level > 1) {
                printf("Calendar tiles:\n");
            }
            render_calendar(cal, cont, &config);

            // Reset the counter
            x = 0;
        }

        // Get the current time
        time(&data.current_time);

        // Redraw the info_bar (maybe a memory problem?)
        render_info_bar(&data, cont, &config);
        lv_timer_handler();

        // Update every minute
        sleep(60);
    } while (1);

    lv_obj_clean(cont);

    return 0;
}

