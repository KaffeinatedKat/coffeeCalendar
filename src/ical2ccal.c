// ical2ccal.c - converts an input ical file to the ccal format and outputs it to stdout
//
// The format is as follows
// %NEvent Name%N %ADYes/No%AD %Dyyyy/mm/dd%D %Bhh:mm%B %Ehh:mm%E %Cfile_name%C
//
// Each event is contained to a single line, and is
// designed to be easily searchable by regex look[ahead/behind]
//
// %N: Event name
// %AD: All day [Yes/No]
// %D: Date of the event
// %B: Event start time
// %E Event end time
// %C iCal file name
//
// %B and %E are not set if %AD is Yes
//
// e.g.
//
// Single timed event
// %NJohn Taco Smell%N %ADNo%AD %D20240304%D %B1700%B %E2200%E %Cjohn_work.ics%C
//
// Single all day event
// %NJohn's Birthday%N %ADYes%AD %D20240922%D %Cbirthdays.ics%C
//
// Multiday events are simply added as multiplie lines
// %NCamping%N %AD%Yes%AD %D20240820%D %Cfamily.ics%C
// %NCamping%N %AD%Yes%AD %D20240821%D %Cfamily.ics%C
// %NCamping%N %AD%Yes%AD %D20240822%D %Cfamily.ics%C
// %NCamping%N %AD%Yes%AD %D20240823%D %Cfamily.ics%C

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libical/ical.h>
#include <time.h>
#include <string.h>

#include "config.h"

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    buffer[file_size] = '\0';
    return buffer;
}


void process_event(icalcomponent *event, const char *calendar_name) {
    // Get the SUMMARY property, which represents the event name
    icalproperty *summary_property = icalcomponent_get_first_property(event, ICAL_SUMMARY_PROPERTY);

    if (summary_property) {
        const char *event_name = icalproperty_get_summary(summary_property);
        printf("%%N%s%%N ", event_name);
    } else {
        printf("%%%s%%N ", PLACEHOLDER_EVENT_NAME);  // Placeholder if event has no name
    }

    // Get the DTSTART property, which represents the start date and time
    icalproperty *start_property = icalcomponent_get_first_property(event, ICAL_DTSTART_PROPERTY);

    if (start_property) {
        struct icaltimetype start_time = icalproperty_get_dtstart(start_property);
        icaltimezone *local_zone = icaltimezone_get_builtin_timezone("localtime");
        time_t epoch_time = icaltime_as_timet_with_zone(start_time, local_zone);

        struct tm *time_info = localtime(&epoch_time);

        // Change event timezone if it's not in the current zone
        if (start_time.zone && strcmp(icaltimezone_get_tzid(start_time.zone), "UTC") == 0) {
            start_time.year = time_info->tm_year + 1900;
            start_time.month = time_info->tm_mon + 1;
            start_time.day = time_info->tm_mday;
            start_time.hour = time_info->tm_hour;
            start_time.minute = time_info->tm_min;
        }

        // Check if it's an all-day event
        if (icaltime_is_date(start_time) != 0) {
            printf("%%ADYes%%AD ");
            printf("%%D%04d/%02d/%02d%%D ", start_time.year, start_time.month, start_time.day);
        } else {
            printf("%%ADNo%%AD ");
            printf("%%D%04d/%02d/%02d%%D ", start_time.year, start_time.month, start_time.day);
            printf("%%B%02d:%02d%%B ", start_time.hour, start_time.minute);
        }
    } else {
        printf("%%ADNo%%AD ");  // Placeholder for missing start date
        printf("%%D0000/00/00%%D ");
    }

    // Get the DTEND property, which represents the end date and time
    icalproperty *end_property = icalcomponent_get_first_property(event, ICAL_DTEND_PROPERTY);

    if (end_property) {
        struct icaltimetype end_time = icalproperty_get_dtend(end_property);
        icaltimezone *local_zone = icaltimezone_get_builtin_timezone("localtime");
        time_t epoch_time = icaltime_as_timet_with_zone(end_time, local_zone);

        struct tm *time_info = localtime(&epoch_time);

        // Change event timezone if it's not in the current zone
        if (end_time.zone && strcmp(icaltimezone_get_tzid(end_time.zone), "UTC") == 0) {
            end_time.hour = time_info->tm_hour;
            end_time.minute = time_info->tm_min;
        }

        // Check if it's an all-day event
        if (icaltime_is_date(end_time) == 0) {
            printf("%%E%02d:%02d%%E ", end_time.hour, end_time.minute);
        }
    } else {
        printf("%%E0000/00/00%%E ");  // Placeholder for missing end date
    }

    printf("%%C%s%%C\n", calendar_name);  // Replace 'file_name' with your actual file name
}

bool is_exclude_date(icalcomponent *event, icaltimetype date) {
    icaltimetype exdate;
    icalproperty *property;

    for (property = icalcomponent_get_first_property(event, ICAL_EXDATE_PROPERTY);
             property != NULL;
             property = icalcomponent_get_next_property(event, ICAL_EXDATE_PROPERTY)) {
        exdate = icalproperty_get_exdate(property);

        if (icaltime_as_timet(exdate) == icaltime_as_timet(date)) {
            return true;
        }
    }
    return false;
}

void add_recurring_events(icalcomponent *ical_root, icalcomponent *event, const char *calendar_name) {
    int max_number_of_events = 1825;
    bool already_exists = false;

    icalproperty *rrule_property = icalcomponent_get_first_property(event, ICAL_RRULE_PROPERTY);
    icalproperty *start_property = icalcomponent_get_first_property(event, ICAL_DTSTART_PROPERTY);
    icalproperty *exdate_property = icalcomponent_get_first_property(event, ICAL_EXDATE_PROPERTY);

    struct icalrecurrencetype rrule = icalproperty_get_rrule(rrule_property);
    struct icaltimetype start_time = icalproperty_get_dtstart(start_property);

    icaltimezone *local_zone = icaltimezone_get_builtin_timezone("localtime");
    time_t epoch_time = icaltime_as_timet_with_zone(start_time, local_zone);

    struct tm *time_info = localtime(&epoch_time);//localtime(&epoch_time);

    // Expand the event out 5 years ahead
    //
    // The function is fucked, if we unput any date that is not
    // the start date for the recurring event then it does not 
    // expand the event properly and we get the wrong dates.
    // We we expand the event 5 years from the original, and
    // ignore anything not within 1 month of the current
    // date
    // This WILL break if the current date is 5 years away
    // from the original start date of the recurring event

    time_t array[1825];
    int count = icalrecur_expand_recurrence(
        icalrecurrencetype_as_string_r(&rrule),
        icaltime_as_timet(start_time),
        max_number_of_events,
        array
    );
    
    for (int i = 0; i < max_number_of_events; ++i) {
        already_exists = false;
        icaltimetype today = icaltime_today();
        struct tm *local_time = localtime(&array[i]);

        icalcomponent *expanded_event = event;
        icaltimetype expanded_event_time = icaltime_from_timet_with_zone(array[i], 0, icaltimezone_get_builtin_timezone(start_time.zone));


        // See line 149
        // Event happens last year
        if (expanded_event_time.year < today.year) {
            continue;
        } else if (expanded_event_time.year > today.year) {
            continue;
        // Event happens more than 1 month backward
        } else if (expanded_event_time.month < today.month - 1) {
            continue;
        // Event happens more than 1 month forward
        } else if (expanded_event_time.month > today.month + 1) {
            break;
        }

        printf("day %d/%d/%d\n", expanded_event_time.year, expanded_event_time.month, expanded_event_time.day);

        icalproperty *start_property = icalcomponent_get_first_property(event, ICAL_DTSTART_PROPERTY);

        // Preserve the all day status of expanded events
        if (icaltime_is_date(icalproperty_get_dtstart(start_property)) != 0) {
            expanded_event_time.is_date = 1;
        }

        // Set the new date for the expanded event
        if (start_property) {
            icalproperty_set_dtstart(start_property, expanded_event_time);
        }

        icalproperty *recurrence_id;

        // Loop through each event to handle modified recurring events
        for (icalcomponent *other_event = icalcomponent_get_first_component(ical_root, ICAL_VEVENT_COMPONENT);
             other_event != NULL;
             other_event = icalcomponent_get_next_component(ical_root, ICAL_VEVENT_COMPONENT)) {
            recurrence_id = icalcomponent_get_first_property(other_event, ICAL_RECURRENCEID_PROPERTY);
            // Check if the event has a recurrence_id and
            if (recurrence_id) {
                struct icaltimetype recurrence_id_time = icalproperty_get_recurrenceid(recurrence_id);

                // If the recurrence_id is the same as the start time of the 
                // expanded event
                if (icaltime_compare(expanded_event_time, recurrence_id_time) == 0) {
                    printf("      uid: %s\nother uid: %s\ncheck: %d\n", icalcomponent_get_uid(event), icalcomponent_get_uid(other_event), strcmp(icalcomponent_get_uid(event), icalcomponent_get_uid(other_event)));
                    // And the UID's are the same
                    if (strcmp(icalcomponent_get_uid(event), icalcomponent_get_uid(other_event)) == 0) {
                        // Then this event has been modified and placed elsewhere,
                        // and we must exclude it to prevent duplicate events
                        already_exists = true;
                    }
                }
            }
        }


        // Add the event to the list if it's not an excluded date
        if (array[i] != 0 && !is_exclude_date(event, expanded_event_time) && !already_exists) {
            process_event(expanded_event, calendar_name);
        } else {
            printf("exclude date!\n");
        }

        icalcomponent_free(expanded_event);
    }
}

void process_events(icalcomponent *ical_root, const char *calendar_name) {
    // Iterate through each VEVENT component in the iCalendar data
    for (icalcomponent *event = icalcomponent_get_first_component(ical_root, ICAL_VEVENT_COMPONENT);
         event != NULL;
         event = icalcomponent_get_next_component(ical_root, ICAL_VEVENT_COMPONENT)) {


        // Get the RRULE property, which represents the recurrence rule
        icalproperty *rrule_property = icalcomponent_get_first_property(event, ICAL_RRULE_PROPERTY);

        // Recurring event
        if (rrule_property) {
            printf("\nThis event repeats\n");
            add_recurring_events(icalparser_parse_string(read_file(calendar_name)), event, calendar_name);
        } else {
            printf("\nThis event does not repeat\n");
            process_event(event, calendar_name);
        }
    }
}


int main(int  argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ical input file>\n", argv[0]);
        return 1;
    }

    char *ical_data = read_file(argv[1]);

    icalcomponent *ical_root = icalparser_parse_string(ical_data);
    if (ical_root == NULL) {
        fprintf(stderr, "Error parsing iCalendar data\n");
        return 1;
    }

    // Process regular events
    process_events(ical_root, argv[1]);

    // Clean up
    free(ical_data);
    icalcomponent_free(ical_root);

    return 0;
}


