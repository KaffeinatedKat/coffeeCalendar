#include "ical2ccal.h"

struct events_status {
    int nprocessed_events;
    int nerror_events;
    int nrenamed_events;
};

struct events_status process_event(struct ccal_calendar *cal, icalcomponent *event, char **blacklist, int blacklist_size, char *calendar_name) {
    // The ccal event to load the ical data into
    struct ccal_event ccal_event;
    struct events_status ret_val = {0};
    struct tm *time_info;
    struct icaltimetype start_time;
    struct icaltimetype end_time;
    icaltimezone *local_zone;
    time_t epoch_time;
    char *endptr;
    const char *event_name;

    // Get the SUMMARY property
    icalproperty *summary_property = icalcomponent_get_first_property(event, ICAL_SUMMARY_PROPERTY);
    if (!summary_property) {
        ret_val.nerror_events++;
        return ret_val;
    }

    event_name = icalproperty_get_summary(summary_property);
    ccal_event.name = event_name;

    // Get the DTSTART property
    icalproperty *start_property = icalcomponent_get_first_property(event, ICAL_DTSTART_PROPERTY);
    if (!start_property) {
        ret_val.nerror_events++;
        return ret_val;
    }

    start_time = icalproperty_get_dtstart(start_property);
    local_zone = icaltimezone_get_builtin_timezone("localtime");
    epoch_time = icaltime_as_timet_with_zone(start_time, local_zone);
    time_info = gmtime(&epoch_time);

    // Change event timezone if it's not in the current zone
    if (start_time.zone && strcmp(icaltimezone_get_tzid((icaltimezone*)start_time.zone), "UTC") == 0) {
        time_info = localtime(&epoch_time);
    }

    // Event date
    ccal_event.date = *time_info;

    // All day / Start time info
    if (icaltime_is_date(start_time) != 0) {
        ccal_event.all_day = 1;
    } else {
        ccal_event.all_day = 0;
        ccal_event.start = *time_info;
    }

    // Get the DTEND property, which represents the end date and time
    icalproperty *end_property = icalcomponent_get_first_property(event, ICAL_DTEND_PROPERTY);
    if (!end_property) {
        ret_val.nerror_events++;
        return ret_val;
    }

    end_time = icalproperty_get_dtend(end_property);
    local_zone = icaltimezone_get_builtin_timezone("localtime");
    epoch_time = icaltime_as_timet_with_zone(end_time, local_zone);
    time_info = gmtime(&epoch_time);

    // Change event timezone if it's not in the current zone
    if (end_time.zone && strcmp(icaltimezone_get_tzid((icaltimezone*)end_time.zone), "UTC") == 0) {
        time_info = localtime(&epoch_time);
    }

    // Check if it's an all-day event
    if (icaltime_is_date(end_time) == 0) {
        ccal_event.end = *time_info;
    }

    // Might not be useful? TODO: Find out
    ccal_event.cal_name = calendar_name;

    // End of the calendar path
    endptr = strchr(calendar_name, '\0');

    // Copy just the number part of the file path
    calendar_name = strndup(endptr - 8, 3);

    // Change it to an int for color indexing
    ccal_event.color_index = strtol(calendar_name, &endptr, 10);

    // Add the event to the event list
    ccal_add_event(cal, ccal_event, blacklist, blacklist_size);
    ret_val.nprocessed_events++;

    return ret_val;
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

struct events_status add_recurring_events(struct ccal_calendar *cal, icalcomponent *ical_root, icalcomponent *event, char **blacklist, int blacklist_size, char *calendar_name) {
    struct events_status ret_val = {0};
    int max_number_of_events = 1825;
    bool already_exists = false;

    icalproperty *rrule_property = icalcomponent_get_first_property(event, ICAL_RRULE_PROPERTY);
    icalproperty *start_property = icalcomponent_get_first_property(event, ICAL_DTSTART_PROPERTY);
    icalproperty *exdate_property = icalcomponent_get_first_property(event, ICAL_EXDATE_PROPERTY);

    struct icalrecurrencetype rrule = icalproperty_get_rrule(rrule_property);
    struct icaltimetype start_time = icalproperty_get_dtstart(start_property);

    icaltimezone *local_zone = icaltimezone_get_builtin_timezone("localtime");
    time_t epoch_time = icaltime_as_timet_with_zone(start_time, local_zone);

    struct tm *time_info = localtime(&epoch_time);

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
        struct events_status tmp_val = {0};

        icalcomponent *expanded_event = event;
        icaltimetype expanded_event_time = icaltime_from_timet_with_zone(array[i], 0, icaltimezone_get_builtin_timezone("localtime"));


        // See line 149
        // Event happens last year
        if (expanded_event_time.year < today.year) {
            continue;
        } else if (expanded_event_time.year > today.year) {
            continue;
        // Event happens more than 1 month backward
        } else if (expanded_event_time.month < today.month - 1) {
            continue;
        // Event happens more than 2 months forward
        } else if (expanded_event_time.month > today.month + 2) {
            break;
        }

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
            // Check if the event has a recurrence_id
            if (!recurrence_id) {
                continue;
            }
            struct icaltimetype recurrence_id_time = icalproperty_get_recurrenceid(recurrence_id);

            // If the recurrence_id is the same as the start time of the 
            // expanded event
            if (icaltime_compare(expanded_event_time, recurrence_id_time) != 0) {
                continue;
            }

            // And the UID's are the same
            if (strcmp(icalcomponent_get_uid(event), icalcomponent_get_uid(other_event)) != 0) {
                continue;
            }
            // Then this event has been modified and placed elsewhere,
            // and we must exclude it to prevent duplicate events
            already_exists = true;
            ret_val.nrenamed_events++;
        }


        // Add the event to the list if it's not an excluded date
        if (array[i] != 0 && !is_exclude_date(event, expanded_event_time) && !already_exists) {
            tmp_val = process_event(cal, expanded_event, blacklist, blacklist_size, calendar_name);
            ret_val.nerror_events += tmp_val.nerror_events;
            ret_val.nprocessed_events += tmp_val.nprocessed_events;
        }

        icalcomponent_free(expanded_event);
    }

    return ret_val;
}

int ical2ccal_load_events(struct ccal_calendar *cal, icalcomponent *ical_root, char *calendar_name, char **blacklist, int blacklist_size, int log_level) {
    struct events_status output = {0};
    int total_events = 0;
    int renamed_duplicates = 0;        
    int events_with_errors = 0;
    char *event_count_color = ANSI_RESET;
    char *corrupt_event_color = ANSI_RESET;

    // Iterate through each VEVENT component in the iCalendar data
    for (icalcomponent *event = icalcomponent_get_first_component(ical_root, ICAL_VEVENT_COMPONENT);
         event != NULL;
         event = icalcomponent_get_next_component(ical_root, ICAL_VEVENT_COMPONENT)) {


        // Get the RRULE property, which represents the recurrence rule
        icalproperty *rrule_property = icalcomponent_get_first_property(event, ICAL_RRULE_PROPERTY);

        // Recurring event
        if (rrule_property) {
            struct file new_ical_root;
            ccal_read_file(&new_ical_root, calendar_name);

            output = add_recurring_events(cal, icalparser_parse_string(new_ical_root.content), event, blacklist, blacklist_size, calendar_name);
        } else {
            output = process_event(cal, event, blacklist, blacklist_size, calendar_name);
        }

        total_events += output.nprocessed_events;
        renamed_duplicates += output.nrenamed_events;
        events_with_errors += output.nerror_events;
    }

    if (total_events == 0) {
        event_count_color = ANSI_RED;
        log_level = 1;
    }
    if (events_with_errors > 0) {
        corrupt_event_color = ANSI_RED;
        log_level = 1;
    }

    if (log_level > 0) {
        printf("Parsing events @ `%s`\n", calendar_name);
        printf("%s[%02d] Successfully parsed events%s\n", event_count_color, total_events, ANSI_RESET);
        printf("[%02d] Renamed duplicates excluded\n", renamed_duplicates);
        printf("%s[%02d] Corrupt events%s\n\n", corrupt_event_color, events_with_errors, ANSI_RESET);
    }
}

