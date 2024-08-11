#ifndef ERROR_H
#define ERROR_H
#include <stdio.h>

#define ANSI_RED "\x1b[31m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET "\x1b[0m"

enum e_errors {
    E_ERROR,
    E_WARNING,
};

void error_log(char *message, int error_type);

#endif
