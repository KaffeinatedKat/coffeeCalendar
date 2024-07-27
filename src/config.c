#include "config.h"
#define ANSI_TERMINAL_RED "\x1b[31m"
#define ANSI_TERMINAL_YELLOW "\x1b[33m"
#define ANSI_TERMINAL_RESET "\x1b[0m"

enum errors {
    NO_ERR,
    NO_OPTION,
    NO_VALUE,
    TYPE_MISMATCH,
    CAL_COLOR_SIZE_MISMATCH,
    NO_OPENING_BRACKET,
    NO_CLOSING_BRACKET,
    QUOTES_SAME,
};

enum types {
    LIST = 0,
    STRING = 1,
    INT = 2,
};

union un {
    int number;
    char *str;
    char **array;
};

struct values {
    union un val; // The value of 'option'
    int type; // The enum type of the value
    int array_size; // The size of the array if appliciable for error handling
    char *option_line; // The parsed 'option' line for printing errors
    char *option; // The name of 'option'
    int err; // Error code if appliciable
};

int is_all_spaces(char *str) {
    bool return_value = true;
    for (int x = 0; x < strlen(str); x++) {
        if (!isspace(str[x])) {
            return_value = false;
            break;
        }
    }
    return return_value;
}

int print_config_error(struct values *values) {
    printf("\n%sThere are errors in your config!%s\n", ANSI_TERMINAL_RED, ANSI_TERMINAL_RESET);
    printf("\n<==========>\n%s\n<==========>\n\n", values->option_line);

    switch (values->err) {
    case NO_ERR:
        printf("%sNO_ERR: This should not happen%s\n", ANSI_TERMINAL_RED, ANSI_TERMINAL_RESET);
        break;
    case NO_OPTION:
        printf("Option '%s' does not exist in the config file!\n", values->option);
        break;
    case NO_VALUE:
        printf("Option has no associated value!\n");
        break;
    case TYPE_MISMATCH:
        printf("Value does not match the expected type!\n");
        break;
    case CAL_COLOR_SIZE_MISMATCH:
        printf("'online_calendars' and 'online_calendar_colors' must have the same number of elements!\n");
        break;
    case NO_OPENING_BRACKET:
        printf("Array has no opening bracket! '['\n");
        break;
    case NO_CLOSING_BRACKET:
        printf("Array has no closing bracket! ']'\n");
        break;
    case QUOTES_SAME:
        printf("String value only has 1 quote!\n");
        break;
    }
}

int get_config_value(struct file *config_list, const char *option, int type, struct values *values) {
    int option_line_len = 0;
    int option_len = strlen(option);
    int x = 0;
    char *option_line;
    char *p1 = config_list->content;
    char *p2;
    char delims[3] = {'[', '\"', '='};
    bool option_exists = false;
    values->option = option;

    // Use strstr to find 'option' and then check the chars
    // before and after to only match exact values
    for (;;) {
        p1 = strstr(p1 + 1, option);
        if (p1 == NULL) { break; }
        else if (p1[-1] == '\n' && p1[option_len] == ' ') { 
            option_exists = true;
            break;
        }
    }
    if (!option_exists) {
        values->option_line = NULL;
        values->err = NO_OPTION;
        print_config_error(values);
        exit(-1);
    }

    // strdup the line with 'option' 
    for (;;) {
        if (p1[x] == '\n' && p1[x - 1] != '\\') { break; }
        option_line_len++;
        x++;
    }
    option_line = strndup(p1, option_line_len);
    values->option_line = strndup(p1, option_line_len);

    // Loop through 'delims' with strchr to find the type of option's value
    for (int x = 0; x < (sizeof(delims) + 1); x++) {
        p2 = strchr(option_line, delims[x]);
        if (p2 == NULL) { continue; }
        if (type != x) { values->err = TYPE_MISMATCH; break; }

        values->err = parse_value(option_line, x, values);
        break;
    }

    if (values->err != NO_ERR) {
        print_config_error(values);
        exit(-1);
    }
    return 0;
}

int parse_value(char *pair, int type, struct values *values) {
    char *p1, *p2, *tok;

    strsep(&pair, "=");
    tok = strsep(&pair, "=");

    switch (type) {
    case LIST:
        char **list = NULL;
        int list_size = 0;
        int list_elements = 0;
        char *p3;
        bool brk = false;
        values->val.array = NULL;

        // Find the first bracket
        p1 = strchr(tok, '[');
        if (p1 == NULL) { return NO_OPENING_BRACKET; }
        p1[0] = '=';
        p3 = p1;

        for (;;) {
            // Pointers between the value to be extracted
            p2 = strchr(p1 + 1, ',');
            if (p2 == NULL) {
                p2 = strchr(p1 + 1, ']');
                if (p2 == NULL) { return NO_CLOSING_BRACKET; }
                brk = true;
            }
            p3 = strndup(p1, p2 - p1);
            p3[0] = '=';
            // Pass the value into parse_value
            values->err = parse_value(p3, 1, values);
            if (values->err != NO_ERR) { return values->err; }

            // Realloc the array
            if (list_size == list_elements) {
                if (list_size == 0) list_size = 2;
                list_size *= 2;
                list = reallocarray(list, list_size, sizeof(char*));
                if (list == NULL) { exit(1); }
            }
            // Append to the array
            list[list_elements] = values->val.str;
            list_elements++;

            // We have parsed the last element
            if (brk) { break; }

            p1 = p2;
        }

        values->val.array = list;
        values->array_size = list_elements;
        values->type = LIST;

        break;
    case STRING:
        char *rev = strdup(tok);
        values->val.str = NULL;
        values->type = STRING;
        p1 = strchr(tok, '\"');
        p2 = strrchr(tok, '\"');

        if (p1 == p2) { return QUOTES_SAME; }

        values->val.str = strndup(p1 + 1, p2 - p1 - 1);
        break;
    case INT:
        char *endptr;
        values->val.number = NULL;
        values->type = INT;

        if (tok == NULL || \
            tok[0] == '\0' || \
            is_all_spaces(tok)) {
            return NO_VALUE;
        }

        values->val.number = strtol(tok, &endptr, 10);
        break;
    }
    return NO_ERR;
}

int config_create(struct config_options *config, char *config_location) {
    struct file config_list;
    struct values values = {0};
    char* endptr;
    int calendar_size;
    int color_size;
    int file;

    file = read_file(&config_list, config_location);

    // Load default config values if no config found
    if (file == -1) {
        printf("%sConfig file not found at %s\nLoading default values...%s\n", ANSI_TERMINAL_YELLOW, config_location, ANSI_TERMINAL_RESET);
        config->screen_width = 1920;
        config->screen_height = 1080;
        config->refresh_time = 30;
        config->current_day_bgcolor = 0xADD8E6;
        return;
    }


    get_config_value(&config_list, "screen_height", INT, &values);
    config->screen_height = values.val.number;

    get_config_value(&config_list, "screen_width", INT, &values);
    config->screen_width = values.val.number;

    get_config_value(&config_list, "cal_refresh_time", INT, &values);
    config->refresh_time = values.val.number;

    get_config_value(&config_list, "current_day_bgcolor", STRING, &values);
    config->current_day_bgcolor = strtol(values.val.str, &endptr, 16);

    get_config_value(&config_list, "online_calendars", LIST, &values);
    config->online_calendars = values.val.array;
    config->calendar_count = values.array_size;
    calendar_size = values.array_size;

    get_config_value(&config_list, "online_calendar_colors", LIST, &values);
    config->calendar_colors = values.val.array;
    color_size = values.array_size;

    if (color_size != calendar_size) {
        values.err = CAL_COLOR_SIZE_MISMATCH;
        print_config_error(&values);
        exit(-1);
    }

    return 0;
}
