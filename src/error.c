#include "error.h"

void error_log(char *message, int error_type) {
    if (error_type == E_ERROR) {
        printf("%s%s%s\n", ANSI_RED, message, ANSI_RESET);
    } else if (error_type == E_WARNING) {
        printf("%s%s%s\n", ANSI_YELLOW, message, ANSI_RESET);
    }
}
