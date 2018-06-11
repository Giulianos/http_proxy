#ifndef REQUEST_PARSER_PRIVATE_H
#define REQUEST_PARSER_PRIVATE_H

#define MAX_METHOD_LEN 7
#define TEMP_PREALLOC 30
#define MAX_TEMP_SIZE 1024

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <request_parser/request_parser.h>

enum request_parser_state {
    METHOD,
    URL,
    VERSION,
    HEADER_NAME,
    HEADER_VALUE,
    BODY,
    SPACES,
    CRS,
};

typedef enum request_parser_state request_parser_state_t;

bool
is_url_char(char c);

bool
is_header_name_char(char c);

bool
is_header_value_char(char c);

bool
is_version_char(char c);

bool
check_request_has_body(const char * method);

bool
check_temp_space(char ** space, size_t index);

bool
check_ipv4(const char * host);

bool
check_ipv6(const char * host);

bool
fill_host_details(host_details_t host);

bool
extract_host_from_target(const char * target, host_details_t host_details);

int
strncmp_case_insensitive(const char * str1, const char * str2, size_t n);


#endif
