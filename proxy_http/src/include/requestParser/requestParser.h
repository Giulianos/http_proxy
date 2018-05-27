#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <myParserUtils/myParserUtils.h>

#define BUFFER_RESTARTED(b) start_state(b)

// El buffer auxiliar es usado si me llega un read del buffer = 0
// y necesito guardar lo que estaba comparando para cuando read != de 0.
#define AUX_BUFFER_SIZE 255

#define VERSION_TEXT_SIZE 3
// HTTP/*.* - 8 caracteres
#define HTTP_TEXT_SIZE 8

// RFC 1123 - GENERAL ISSUES
#define HOST_MAX_SIZE 255

// El puerto por default es el 80.
#define DEFAULT_PORT 80

typedef enum {
	METHOD,
	URI,
	RELATIVE_URI,
	URI_HOST,
	VERSION,
	START_LINE_END,
	LOCALHOST_HEADER_CHECK,
	HEADERS,
	HOST,
	FINISHED
} requestParserState;

// V_2.0 no es soportado.
typedef enum {
	UNDEFINED, V_1_0, V_1_1
} httpVersion;

typedef enum {
	UNDEFINED_M, GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE
} httpMethod;

typedef enum {
	OK, GENERAL_ERROR,
	VERSION_ERROR, START_LINE_END_ERROR,
	GENERAL_METHOD_ERROR, UNSUPPORTED_METHOD_ERROR,
	HOST_ERROR,
	ALLOCATION_ERROR
} requestState;

typedef struct RequestData {
	requestParserState parserState;
	bool isBufferEmpty;
	requestState state;
	httpVersion version;
	httpMethod method;
	char host[HOST_MAX_SIZE];
	int port;
	bool isLocalHost;
} RequestData;

void defaultRequestStruct (RequestData *rData);
bool checkRequest (requestState *state, buffer *b, buffer *bOut);
const char * errorMessage (const requestState state);

#endif
