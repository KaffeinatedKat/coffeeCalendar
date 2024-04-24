#include "calendar_utils.h"

uint16_t get_week_number(uint16_t year, uint8_t month, uint8_t day) {
    struct tm timeinfo = { .tm_year = year - 1900, .tm_mon = month - 1, .tm_mday = day };
    time_t t = mktime(&timeinfo);
    struct tm *tm = localtime(&t);
    char buf[10];

    strftime(buf, sizeof(buf), "%U", tm);
    return atoi(buf) + 1; // Adding 1 because week numbers start from 0
}

uint8_t day_of_week(uint16_t year, uint8_t month, uint8_t day) {
    return ((day += month < 3 ? year-- : year - 2, 23*month/9 + day + 4 + year/4- year/100 + year/400)%7);
}

bool is_leap_year(uint16_t year) {
    // Leap year if divisible by 4
    if (year % 4 != 0) {
        return 0; // Not a leap year
    }
    // If divisible by 4 but not by 100, or divisible by 400, it's a leap year
    else if (year % 100 != 0 || year % 400 == 0) {
        return 1; // Leap year
    }
    // If divisible by 100 but not by 400, it's not a leap year
    else {
        return 0; // Not a leap year
    }
}

uint8_t last_day_of_month(uint8_t month, bool leap_year) {
    switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        if (leap_year) {
            return 29;
        }
        return 28;
    default:
        return -1;
    }
}

char *month_name(uint8_t month) {
    static char name[4];
    switch (month) {
        case 1:
            strcpy(name, "Jan");
            break;
        case 2:
            strcpy(name, "Feb");
            break;
        case 3:
            strcpy(name, "Mar");
            break;
        case 4:
            strcpy(name, "Apr");
            break;
        case 5:
            strcpy(name, "May");
            break;
        case 6:
            strcpy(name, "Jun");
            break;
        case 7:
            strcpy(name, "Jul");
            break;
        case 8:
            strcpy(name, "Aug");
            break;
        case 9:
            strcpy(name, "Sep");
            break;
        case 10:
            strcpy(name, "Oct");
            break;
        case 11:
            strcpy(name, "Nov");
            break;
        case 12:
            strcpy(name, "Dec");
            break;
        default:
            strcpy(name, "FUK");
            break;
    }
    return name;
}

char *week_name(uint8_t week_day) {
    static char name[4];

    switch (week_day) {
    case 0:
        strcpy(name, "SUN");
        break;
    case 1:
        strcpy(name, "MON");
        break;
    case 2:
        strcpy(name, "TUE");
        break;
    case 3:
        strcpy(name, "WED");
        break;
    case 4:
        strcpy(name, "THU");
        break;
    case 5:
        strcpy(name, "FRI");
        break;
    case 6:
        strcpy(name, "SAT");
        break;
    default:
        strcpy(name, "FUK");
        break;
    }
    return name;
}
