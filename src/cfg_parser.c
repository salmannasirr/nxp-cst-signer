#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cfg_parser.h>

#define DELIMITER "="
#define FREE(p) do { if(p) free(p); } while(0)

/* Trim leading and trailing spaces, tabs, and newlines */
char *trim(char *s) {
    if (!s) return NULL;

    while (*s == ' ' || *s == '\t') s++;          // leading spaces
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        *end-- = '\0';                            // trailing spaces/newlines

    return s;
}

/* Extract value corresponding to exp_key in line */
char *get_value(char *line, const char *exp_key) {
    if (!line || !exp_key) return NULL;

    char *line_copy = strdup(line); 
    if (!line_copy) return NULL;

    char *key = strtok(line_copy, DELIMITER);
    if (!key) {
        FREE(line_copy);
        return NULL;
    }

    key = trim(key);
    if (strncmp(key, exp_key, strlen(exp_key)) == 0) {
        char *val = line + strlen(key);  // start at '='
        while (*val == '=' || *val == ' ' || *val == '\t') val++;  // skip '=' and spaces/tabs

        /* Strip trailing inline comment (# or //) */
        char *comment = strstr(val, "#");
        if (!comment) comment = strstr(val, "//");
        if (comment) *comment = '\0';

        val = trim(val);  
        FREE(line_copy);
        return val;
    }

    FREE(line_copy);
    return NULL;
}

/* Parse configuration file for exp_key and copy result to res_val */
void cfg_parser(FILE *fp_cfgfile, char *res_val, unsigned int res_size, char *exp_key) {
    if (!fp_cfgfile || !res_val || !exp_key) return;

    char *line_buf = NULL;
    size_t length = 0;
    ssize_t read_len;
    char *res;

    memset(res_val, 0, res_size);
    rewind(fp_cfgfile);

    while (-1 != (read_len = getline(&line_buf, &length, fp_cfgfile))) {
        char *line = trim(line_buf);

        /* Skip empty lines, comments, and section headers */
        if (!line[0] || line[0] == '#' || (line[0] == '/' && line[1] == '/') || line[0] == '[')
            continue;

        res = get_value(line, exp_key);
        if (res) {
            strncpy(res_val, res, res_size - 1);
            res_val[res_size - 1] = '\0';
            break;
        }
    }

    FREE(line_buf);
}
