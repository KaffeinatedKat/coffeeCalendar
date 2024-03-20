#include <stdlib.h>
#include <stdio.h>
#include "ccal.h"

int main() {
    char *reggy = "%N(.*?)%N";
    char *string = "%NJohn's Driving Test%N %ADNo%AD %D2024/03/08%D %B15:30%B %E16:30%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/02/02%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/02/09%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/02/16%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/02/23%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/03/01%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/03/08%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/03/15%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/03/22%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/03/29%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/04/05%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/04/12%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/04/19%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NThis event repeats forever%N %ADNo%AD %D2024/04/26%D %B09:30%B %E10:00%E %Ctest.ics%C\n%NSome event lmao%N %ADYes%AD %D2024/03/19%D %Ctest.ics%C\n%NThis one is long%N %ADNo%AD %D2024/03/20%D %B12:00%B %E22:00%E %Ctest.ics%C";
    //char *string = "%NJohn's Driving Test%N %ADNo%AD %D2024/03/08%D %B15:30%B %E16:30%E %Ctest.ics%C";
    //char *output = get_event_name(string);
    char *output = get_events(string, 8, 3, 2024);

    printf("output: %s\n", output);

    free(output);
}
