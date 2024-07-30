#ifndef ERROR_H
#define ERROR_H
#include <stdio.h>

enum e_errors {
    E_ERROR,
    E_WARNING,
};

void error_log(char *message, int error_type);

#endif
