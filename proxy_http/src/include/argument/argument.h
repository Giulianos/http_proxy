#ifndef PROXY_HTTP_ARGUMENT_H
#define PROXY_HTTP_ARGUMENT_H

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string/mystring.h>
#include <errno.h>

#define VERSION "0.0.0"

typedef enum {ARG_OK, MEMERR} arg_res_type;

struct mediatypes{
    int size;
    char** types;
};


#endif //PROXY_HTTP_ARGUMENT_H
