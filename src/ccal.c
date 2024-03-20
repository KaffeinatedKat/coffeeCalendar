#include "ccal.h"

char *regex_parse(char *input_string, char *regex_string) {
    regex_t compiled_regex;
    regmatch_t matches[2];
    char *matched_string;
    uint32_t matched_string_len;
    char *return_string;

    if (regcomp(&compiled_regex, regex_string, REG_EXTENDED | REG_NEWLINE) != 0) {
        return "Invalid regex string";
    }

    while (true) {
        if (regexec(&compiled_regex, input_string, 2, matches, 0) != 0) {
            return matched_string;
        }

        if (matches[1].rm_so != -1) {
            matched_string = input_string + matches[1].rm_so;
            matched_string_len = matches[1].rm_eo - matches[1].rm_so;

            return_string = malloc(matched_string_len);

            snprintf(return_string, matched_string_len + 1, "%s\n", matched_string);
            return return_string;
        }

        input_string += matches[0].rm_eo;
    }
}

FILE *get_events(char *file_path, uint16_t year, uint8_t month, uint8_t day) {
    FILE *command_output;
    static char *command;

    command = malloc(4069);

    // Hell itself
    // Jankily uses shell commands to print out all the events for a given date,
    // sorting them by start time
    sprintf(command, "cat %s | grep %d/%02d/%02d | awk -F'[%%]' '{print $0,$6}' | sort -k7 | awk '{print $0}'", file_path, year, month, day);

    command_output = popen(command, "r");
    free(command);
    return command_output;
}

char *get_event_name(char *event) {
    char *name = regex_parse(event, "%N(.*?)%N");
    return name;
}

char *get_event_calendar_name(char *event) {
    char *name = regex_parse(event, "%C(.*?)%C");
    return name;
}

char *get_event_start_time(char *event) {
    char *start_time = regex_parse(event, "%B(.*?)%B");
    return start_time;
}

char *get_event_end_time(char *event) {
    char *end_time = regex_parse(event, "%E(.*?)%E");
    return end_time;
}

bool get_event_all_day(char *event) {
    char *all_day = regex_parse(event, "%AD(.*?)%AD");
    if (strcmp(all_day, "Yes") == 0) {
        free(all_day);
        return true;
    }
    return false;
}

uint16_t get_number_of_events(FILE *ccal_event_list) {
    int64_t event_count = 0;
    int c;

    while ((c = fgetc(ccal_event_list)) != EOF) {
        if (c == '\n') {
            event_count++;
        }
    }
    return event_count;
}

// Loop through all 7 days of the specified week number and get the maximum number of events
// for any of the days
uint16_t get_max_events_for_week(uint16_t year, uint8_t month, uint8_t day) {
    struct tm timeinfo = {.tm_year = year - 1900, .tm_mon = 0, .tm_mday = 1};
    time_t t = mktime(&timeinfo);
    FILE *events;
    uint16_t week_number = get_week_number(year, month, day);
    int32_t event_count;
    int32_t max_events = 0;

    timeinfo = *localtime(&t);
    timeinfo.tm_mday -= 1;
    t = mktime(&timeinfo);
    // Magic date math
    t += (week_number - 1) * 7 * 24 * 60 * 60;

    for (int i = 0; i < 7; i++) {
        struct tm *tm = localtime(&t);

        events = get_events(CCAL_LOCATION, year, month, tm->tm_mday);
        event_count = get_number_of_events(events);
        if (event_count > max_events) {
            max_events = event_count;
        }

        // Move to the next day
        t += 24 * 60 * 60;
    }

    return max_events;
}
