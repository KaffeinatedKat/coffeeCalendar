#include "calendar_utils.h"

uint8_t first_day_of_week(uint16_t year, uint8_t month, uint8_t day) {
    // Zeller's Congruence algorithm
    if (month < 3) {
        month += 12;
        year -= 1;
    }

    int q = day;
    int m = month;
    int K = year % 100;
    int J = year / 100;

    int h = (q + (13 * (m + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    return (h + 6) % 7;
}

const char *month_name(uint8_t month) {
    const char *name;
    switch (month) {
        case 1:
    	    name = "Jan";
            break;
        case 2:
    	    name = "Feb";
            break;
        case 3:
            name = "Mar";
            break;
        case 4:
            name = "Apr";
            break;
        case 5:
            name = "May";
            break;
        case 6:
            name = "Jun";
            break;
        case 7:
            name = "Jul";
            break;
        case 8:
	        name = "Aug";
            break;
        case 9:
	        name = "Sep";
            break;
        case 10:
	        name = "Oct";
            break;
        case 11:
            name = "Nov";
            break;
        case 12:
            name = "Dec";
            break;
        default:
            name = "FUK";
            break;
    }
    return name;
}

const char *week_name(uint8_t week_day) {
    const char *name;

    switch (week_day) {
    case 0:
        name = "SUN";
        break;
    case 1:
        name = "MON";
        break;
    case 2:
        name = "TUE";
        break;
    case 3:
        name = "WED";
        break;
    case 4:
        name = "THU";
        break;
    case 5:
        name = "FRI";
        break;
    case 6:
        name = "SAT";
        break;
    default:
        name = "FUK";
        break;
    }
    return name;
}
