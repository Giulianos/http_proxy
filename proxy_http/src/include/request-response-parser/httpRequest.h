#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include "parserUtils.h"

#define VERSION_TEXT_SIZE 3
// HTTP/*.* - 8 caracteres
#define HTTP_TEXT_SIZE 8

// RFC 1123 - GENERAL ISSUES
#define HOST_MAX_SIZE 255

// V_2.0 no es soportado.
typedef enum {
	UNDEFINED, V_1_0, V_1_1
} httpVersion;

typedef enum {
	UNDEFINED_M, GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE
} httpMethod;

typedef enum {
	OK, GENERAL_ERROR,
	START_LINE_FORMAT_ERROR, METHOD_ERROR, VERSION_ERROR,
	HOST_ERROR,
	ALLOCATION_ERROR
} requestState;

typedef struct RequestData {
	requestState state;
	httpVersion version;
	httpMethod method;
	char host[HOST_MAX_SIZE];
} RequestData;

void defaultRequestStruct (RequestData *rData);
bool checkRequest (requestState *state, buffer *b);
const char * errorMessage (const requestState state);

#endif
