#ifndef CAL_UTILS
#define CAL_UTILS
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

uint8_t calutils_first_day_of_week(uint16_t year, uint8_t month, uint8_t day);
const char *calutils_month_name(uint8_t month);
const char *calutils_week_name(uint8_t week_day);

#endif
