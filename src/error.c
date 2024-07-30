#include "error.h"

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET "\x1b[0m"

void error_log(char *message, int error_type) {
    if (error_type == E_ERROR) {
        printf("%s%s%s\n", ANSI_RED, message, ANSI_RESET);
    } else if (error_type == E_WARNING) {
        printf("%s%s%s\n", ANSI_YELLOW, message, ANSI_RESET);
    }
}
