#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <myParserUtils/myParserUtils.h>

// Códigos de status
#define STATUS_OK 200
#define STATUS_NO_CONTENT 204

#define NO_BODY_LENGTH -1

#define VERSION_TEXT_SIZE 3

typedef enum {
	RES_SPACE_TRANSITION,
	RES_VERSION,
	STATUS,
	RES_HEADERS,
	LENGTH_CHECK,
	ENCODING_CHECK,
	CHUNKED_CHECK,
	CONNECTION_CHECK,
	TYPE_CHECK,
	BODY_NORMAL,
	BODY_TRANSFORMATION,
	RES_FINISHED
} responseParserState;

typedef enum {
	IDENTITY, GZIP
} contentEncoding;

typedef enum {
	RES_OK, RES_GENERAL_ERROR,
	START_LINE_FORMAT_ERROR, RES_VERSION_ERROR, STATUS_ERROR,
	HEADERS_END_ERROR,
	CHUNKS_ERROR,
	RES_ALLOCATION_ERROR
} responseState;

typedef struct ResponseData {
	responseParserState parserState;
	responseParserState next;
	bool isBufferEmpty;
	responseState state;
	httpVersion version;
	int status;
	bool isClose; // Veo si ya puse el header Connection: close. (no soporto conexiones persistentes)
	int bodyLength; // Si estoy en modo chunk lo uso para cada chunk.
	contentEncoding cEncoding;
	bool isChunked;
	bool withTransf; // Con transformación.
} ResponseData;

void defaultResponseStruct (ResponseData *rData);
bool checkResponse (ResponseData *rd, buffer *b, buffer *bOut, buffer *bTransf);

#endif
