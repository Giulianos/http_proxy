#ifndef PROXY_HTTP_ARGUMENT_H
#define PROXY_HTTP_ARGUMENT_H

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define VER_ARG "0.0.0"

/**
* @param argc and argv from main
* initializes the configurations received from parameters
*/
void argument_get(int argc, char** argv);

#endif // PROXY_HTTP_ARGUMENT_H
