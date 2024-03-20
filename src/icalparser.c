// icalParser.c - An iCal parser for coffeeCalendar
// <----->
// Reads through iCal files and converts them to the 
// coffeeCalendar (.ccal) file format
//
// Each event is contained to a single line, and is
// designed to be easily searchable by regex lookahead/behind
//
// The format is as follows
// %NEvent Name%N %ADYes/No%AD %Dyyyy/mm/dd%D %Bhh:mm%B %Ehh:mm%E %Cfile_name%C
//
// %N: Event name
// %AD: All day [Yes/No]
// %D: Date of the event
// %B: Event start time
// %E Event end time
// %C iCal file name
//
// %B and %E are not used if %AD is Yes
//
// e.g.
//
// Single timed event
// %NJohn Taco Smell%N %ADNo%AD %D20240304%D %B1700%B %E2200%E %Cjohn_work%C
//
// Single all day event
// %NJohn's Birthday%N %ADYes%AD %D20240922%D %Cbirthdays%C
//
// Multiday events are simply added as multiplie lines
// %NCamping%N %AD%Yes%AD %D20240820%D %CFamily%C
// %NCamping%N %AD%Yes%AD %D20240821%D %CFamily%C
// %NCamping%N %AD%Yes%AD %D20240822%D %CFamily%C
// %NCamping%N %AD%Yes%AD %D20240823%D %CFamily%C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 4096

void splitSegments(FILE *file) {
    char event_name[64];
    char event_start_time[64];
    char event_end_time[64];
    char event_start_date[64];
    char event_end_date[64];
    char calendar_name[64] = "test_cal";
    bool all_day = false;

    char line[MAX_LINE_LENGTH];
    char segment[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) != NULL) {
        // Segments begin with "BEGIN:"
        if (strncmp(line, "BEGIN:", 6) == 0) {
            all_day = false;
            // Continue reading lines until the corresponding "END:" is found
            do {
                fgets(line, sizeof(line), file);


                // Event name
                if (strncmp(line, "SUMMARY:", 8) == 0) {
                    strcpy(event_name, &line[8]);
                    event_name[strcspn(event_name, "\r\n")] = 0;
                // Event start time
                } else if (strncmp(line, "DTSTART;TZID=America/Denver:", 28) == 0) {
                    // Start date
                    memcpy(event_start_date, &line[28], 8);
                    event_start_date[8] = '\0';

                    memcpy(event_start_time, &line[28 + 9], 4);
                    event_start_time[4] = '\0';
                } else if (strncmp(line, "DTEND;TZID=America/Denver:", 26) == 0) {
                    // End date
                    memcpy(event_start_date, &line[26], 8);
                    event_start_date[8] = '\0';

                    memcpy(event_end_time, &line[26 + 9], 4);
                    event_end_time[4] = '\0';
                }
            } while (strncmp(line, "END:", 4) != 0);

            printf("\n%%N%s%%N %%AD%s%%AD %%D%s%%D %%B%s%%B %%E%s%%E %%C%s%%C", event_name, all_day ? "Yes" : "No", event_start_date, event_start_time, event_end_time, calendar_name);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ics_file>\n", argv[0]);
        return 1;
    }

    FILE *icsFile = fopen(argv[1], "r");
    if (icsFile == NULL) {
        perror("Error opening .ics file");
        return 1;
    }

    splitSegments(icsFile);

    fclose(icsFile);
    return 0;
}

