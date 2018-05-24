#include "responseParser.h"

static bool checkResponseInner (ResponseData *rData, buffer *b, buffer *bOut);
// Prototipos Start Line
static bool checkStartLine (ResponseData *rData, buffer *b);
static bool extractHttpVersion (ResponseData *rData, buffer *b);
static bool extractStatus (ResponseData *rData, buffer *b);
static bool isValidStatus (const int status);
// Prototipos Header
static bool checkHeaders (ResponseData *rData, buffer *b);
static bool checkContentHeader (ResponseData *rData, buffer *b);
static bool checkContentLength (ResponseData *rData, buffer *b);
static bool checkContentEncoding (ResponseData *rData, buffer *b);
static bool checkTransferHeader (ResponseData *rData, buffer *b);
static bool checkIfChunked (ResponseData *rData, buffer *b);
// Prototipos Body
static bool extractBody (ResponseData *rData, buffer *b, buffer *bOut);
static bool extractChunkedBody (ResponseData *rData, buffer *b, buffer *bOut);


void defaultResponseStruct (ResponseData *rData) {
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->status = 0;
	rData->bodyLength = -1;
	rData->isChunked = false;
}

bool checkResponse (responseState *state, buffer *b, buffer *bOut) {
	bool success = true;
	// Reservo suficiente memoria para 1 ResponseData struct.
	ResponseData *rData = (ResponseData *) malloc(sizeof(ResponseData));

	if (rData == NULL) {
		rData->state = ALLOCATION_ERROR;
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	success = checkResponseInner(rData, b, bOut);

	if (success == false) {
		*state = (rData->state == OK ?
			GENERAL_ERROR : rData->state);
	}

	free(rData);

	return success;
}

static bool checkResponseInner (ResponseData *rData, buffer *b, buffer *bOut) {
	bool success = true;

	defaultResponseStruct(rData);

	if (success && !checkStartLine(rData, b)) {
		success = false;
	}

	if (success && !checkHeaders(rData, b)) {
		rData->state = HEADERS_END_ERROR;
		success = false;
	}

	if (success && !extractBody(rData, b, bOut)) {
		success = false;
	}

	return success;
}

/**               COMIENZO FUNCIONES DE START LINE              **/

static bool checkStartLine (ResponseData *rData, buffer *b) {
	if (!matchFormat("HTTP/", b)) {
		rData->state = START_LINE_FORMAT_ERROR;
		return false;
	}

	if (!extractHttpVersion(rData, b)) {
		rData->state = VERSION_ERROR;
		return false;
	}

	if (!extractStatus(rData, b)) {
		return false;
	}

	return true;
}

static bool extractHttpVersion (ResponseData *rData, buffer *b) {
	char buf[VERSION_TEXT_SIZE + 1] = {0}; // Reservo espacio para NULL termination.
	char *versionOption[] = {"1.0", "1.1"};
	httpVersion versionType[] = {V_1_0, V_1_1};
	int versions = sizeof(versionType) / sizeof(versionType[0]);

	if (!writeToBuf(buf, VERSION_TEXT_SIZE, b)) {
		return false;
	}

	for (int i = 0; i < versions; i++) {
		if (strcmp(versionOption[i], buf) == 0) {
			rData->version = versionType[i];
			break;
		}
	}

	return rData->version != UNDEFINED;
}

static bool extractStatus (ResponseData *rData, buffer *b) {
	int status = 0;

	moveThroughSpaces(b);

	if (!getNumber(&status, b)) {
		rData->state = START_LINE_FORMAT_ERROR;
		return false;
	}

	rData->status = status;

	if (!isValidStatus(status)) {
		rData->state = STATUS_ERROR;
		return false;
	}

	return true;
}

static bool isValidStatus (const int status) {
	return status == STATUS_OK;
}

/**               FIN DE FUNCIONES DE START LINE                **/

/**               COMIENZO FUNCIONES DE HEADER                  **/

static bool checkHeaders (ResponseData *rData, buffer *b) {
	char c;
	bool endOfHeaders = false;

	while ((c = READ_UP_CHAR(b)) != 0) {
		if (c == 'C') {
			checkContentHeader(rData, b);
		} else if (c == 'T') {
			checkTransferHeader(rData, b);
		} else if (c == '\r') {
			if (checkLF(b) && checkCRLF(b)) {
				// Verifico si hay 2 CRLF consecutivos.
				endOfHeaders = true;
				break;
			}
		}
	}

	return endOfHeaders;
}

static bool checkContentHeader (ResponseData *rData, buffer *b) {
	char c;
	bool result = false;

	if (!matchFormat("ONTENT-", b)) {
		return false;
	}

	if ((c = READ_UP_CHAR(b)) == 'L') {
		result = checkContentLength(rData, b);
	} else if (c == 'E') {
		result = checkContentEncoding(rData, b);
	}

	return result;
}

static bool checkContentLength (ResponseData *rData, buffer *b) {
	int length = 0;

	if (!matchFormat("ENGTH:", b)) {
		return false;
	}

	moveThroughSpaces(b);

	if (!getNumber(&length,b)) {
		return false;
	}

	rData->bodyLength = length;

	return true;
}

static bool checkContentEncoding (ResponseData *rData, buffer *b) {
	if (!matchFormat("NCODING:", b)) {
		return false;
	}

	return checkIfChunked(rData, b);
}

static bool checkTransferHeader (ResponseData *rData, buffer *b) {
	if (!matchFormat("RANSFER-ENCODING:", b)) {
		return false;
	}

	return checkIfChunked(rData, b);
}

static bool checkIfChunked (ResponseData *rData, buffer *b) {
	moveThroughSpaces(b);

	if (!matchFormat("CHUNKED", b)) {
		return false;
	}

	rData->isChunked = true;

	return true;
}

/**               FIN DE FUNCIONES DE HEADER                    **/

/**               COMIENZO FUNCIONES DE BODY                    **/

static bool extractBody (ResponseData *rData, buffer *b, buffer *bOut) {
	// Chunked encoding Sobreescribe content length.
	if (rData->isChunked == true) {
		return extractChunkedBody(rData, b, bOut);
	}

	if (rData->bodyLength > 0) {
		return writeToTransfBuf(b, bOut, rData->bodyLength);
	}

	return true; // No hice nada por lo que no hubo extracción fallida.
}

static bool extractChunkedBody (ResponseData *rData, buffer *b, buffer *bOut) {
	int chunkLength = 0;

	// Con el do while, la última iteración corresponde a dos
	// empty line consecutivos como corresponde.
	do {
		if (!getHexNumber(&chunkLength,b)) {
			rData->state = CHUNK_HEX_ERROR;
			return false;
		}

		if (!checkCRLF(b)) { // Delimitador de longitud de chunk.
			rData->state = CHUNK_DELIMITER_ERROR;
			return false;
		}

		if (!writeToTransfBuf(b, bOut, chunkLength)) {
			return false;
		}

		if (!checkCRLF(b)) { // Delimitador de chunk.
			rData->state = CHUNK_DELIMITER_ERROR;
			return false;
		}
	} while (chunkLength != 0);

	return true;
}

/**               FIN DE FUNCIONES DE BODY                       **/
