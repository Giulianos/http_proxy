#ifndef PROXY_HTTP_ARGUMENT_H
#define PROXY_HTTP_ARGUMENT_H

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#define VER_ARG "0.0.0"

/**
* @param argc and argv from main
* initializes the configurations received from parameters
*/
void argument_get(int argc, char **argv);

#endif //PROXY_HTTP_ARGUMENT_H
