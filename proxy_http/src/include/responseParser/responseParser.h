#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include "myParserUtils.h"

// Códigos de status
#define STATUS_OK 200

#define VERSION_TEXT_SIZE 3

// V_2.0 is not supported.
typedef enum {
	UNDEFINED, V_1_0, V_1_1
} httpVersion;

typedef enum {
	OK, GENERAL_ERROR,
	START_LINE_FORMAT_ERROR, VERSION_ERROR, STATUS_ERROR,
	HEADERS_END_ERROR,
	CHUNK_HEX_ERROR, CHUNK_DELIMITER_ERROR,
	ALLOCATION_ERROR
} responseState;

typedef struct ResponseData {
	responseState state;
	httpVersion version;
	int status;
	int bodyLength;
	bool isChunked;
} ResponseData;

void defaultResponseStruct (ResponseData *rData);
bool checkResponse (responseState *state, buffer *b, buffer *bOut);
const char * errorMessage (const responseState state);

#endif
