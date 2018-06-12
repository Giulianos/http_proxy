#ifndef RESPONSE_PARSER_PRIVATE_H
#define RESPONSE_PARSER_PRIVATE_H

#include <stdlib.h>
#include <stdbool.h>

#define TEMP_PREALLOC 30
#define MAX_TEMP_SIZE 1024

enum response_parser_state {
    VERSION,
    CODE,
    TEXT,
    RES_HEADER_NAME,
    RES_HEADER_VALUE,
    BODY,
    CHUNK_SIZE,
    CHUNK_EXT,
    CHUNK_BODY,
    SPACES,
    CRS,
};

typedef enum response_parser_state response_parser_state_t;

bool
res_check_temp_space(char ** space, size_t index);

bool
res_is_version_char(char c);

bool
res_is_text_char(char c);

bool
res_is_header_name_char(char c);

bool
res_is_header_value_char(char c);

bool
res_check_response_has_body(char * code);

int
res_strncmp_case_insensitive(const char * str1, const char * str2, size_t n);

#endif