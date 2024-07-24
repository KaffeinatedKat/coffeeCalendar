#ifndef CAL_UTILS
#define CAL_UTILS
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

uint16_t get_week_number(uint16_t year, uint8_t month, uint8_t day);
uint8_t day_of_week(uint16_t year, uint8_t month, uint8_t day);
uint8_t last_day_of_month(uint8_t month, bool leap_year);

bool is_leap_year(uint16_t year);

const char *month_name(uint8_t month);
const char *week_name(uint8_t week_day);
#endif
