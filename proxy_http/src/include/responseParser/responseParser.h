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

#define NO_BODY_LENGTH -1

#define VERSION_TEXT_SIZE 3

typedef enum {
	SPACE_TRANSITION,
	VERSION,
	STATUS,
	HEADERS,
	LENGTH_CHECK,
	ENCODING_CHECK,
	TYPE_CHECK,
	BODY_NORMAL,
	BODY_TRANSFORMATION,
	FINISHED
} responseParserState;

// V_2.0 is not supported.
typedef enum {
	UNDEFINED, V_1_0, V_1_1
} httpVersion;

typedef enum {
	OK, GENERAL_ERROR,
	START_LINE_FORMAT_ERROR, VERSION_ERROR, STATUS_ERROR,
	HEADERS_END_ERROR,
	CHUNKS_ERROR,
	ALLOCATION_ERROR
} responseState;

typedef struct ResponseData {
	responseParserState parserState;
	responseParserState next;
	bool isBufferEmpty;
	responseState state;
	httpVersion version;
	int status;
	int bodyLength; // Si estoy en modo chunk lo uso para cada chunk.
	bool isChunked;
	bool withTransf; // Con transformación.
} ResponseData;

/** Seteo la estructura con los valores por default previos a la primer utilización del parser. */
void
defaultResponseStruct (ResponseData *rData);

/** Función que llamo para correr el parser por primera vez. Internamente tiene una función
* a la cual puedo despertar cada vez que tengo algo en el buffer de entrada hasta encontrar al host.
*/
bool
checkResponse (responseState *state, buffer *b, buffer *bOut, buffer *bTransf);

#endif
