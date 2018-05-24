#include "requestParser.h"

static bool checkRequestInner (RequestData *rData, buffer *b);
static void checkLocalhost (RequestData *rData);
// Prototipos Start Line
static bool checkStartLine (RequestData *rData, buffer *b);
static bool extractHttpMethod (RequestData *rData, buffer *b);
static bool checkUri (RequestData *rData, buffer *b);
static void cleanRelativeUri (RequestData *rData, buffer *b);
static bool checkUriForHost (RequestData *rData, buffer *b);
static bool extractHttpVersion (RequestData *rData, buffer *b);
// Prototipos Header
static bool checkHostHeader (RequestData *rData, buffer *b);
static bool extractHost (RequestData *rData, buffer *b);


void defaultRequestStruct (RequestData *rData) {
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->method = UNDEFINED_M;
	rData->isLocalHost = false;

	for (int i = 0; i < HOST_MAX_SIZE; i++) {
		rData->host[i] = 0;
	}
}

bool checkRequest (requestState *state, buffer *b) {
	bool success = true;

	// Reservo suficiente memoria para 1 RequestData struct.
	RequestData *rData = (RequestData *) malloc(sizeof(RequestData));

	if (rData == NULL) {
		rData->state = ALLOCATION_ERROR;
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	success = checkRequestInner(rData, b);

	if (success == false) {
		*state = (rData->state == OK ?
			GENERAL_ERROR : rData->state);
	}

	free(rData);

	return success;
}

static bool checkRequestInner (RequestData *rData, buffer *b) {
	bool success = true;

	defaultRequestStruct(rData);

	if (success && !checkStartLine(rData, b)) {
		success = false;
	}

	// Si ya encontré el host en el uri no hace falta parsear los headers.
	if (success && rData->host[0] != 0) {
		return true;
	}

	if (success && !checkHostHeader(rData, b)) {
		rData->state = HOST_ERROR;
		success = false;
	}

	checkLocalhost(rData);

	return success;
}

static void checkLocalhost (RequestData *rData) {
	char *host = rData->host;

	if (strcmp("localhost", host)) {
		rData-> isLocalHost = true;
	}
	// Falta caso en que paso ip.
}

/**               COMIENZO FUNCIONES DE START LINE              **/

static bool checkStartLine (RequestData *rData, buffer *b) {
	if (!extractHttpMethod(rData, b)) {
		if (rData->method == UNDEFINED_M) {
			rData->state = GENERAL_METHOD_ERROR;
		} else {
			rData->state = UNSUPPORTED_METHOD_ERROR;
		}
		return false;
	}

	moveThroughSpaces(b);

	// Si checkUri es falso pero no copié el host sigo adelante.
	// El problema lo tengo si copié host pero este es inválido.
	if (!checkUri(rData, b) && rData->host[0] != 0) {
		rData->state = HOST_ERROR;
		return false;
	}

	moveThroughSpaces(b);

	if (!matchFormat("HTTP/", b)) {
		rData->state = START_LINE_FORMAT_ERROR;
		return false;
	}

	if (!extractHttpVersion(rData, b)) {
		rData->state = VERSION_ERROR;
		return false;
	}

	return true;
}

static bool extractHttpMethod (RequestData *rData, buffer *b) {
	char *methodOption[] = {"CONNECT", "DELETE", "GET", "HEAD", "OPTIONS", "POST", "PUT", "TRACE"};
	httpMethod methodType[] = {CONNECT, DELETE, GET, HEAD, OPTIONS, POST, PUT, TRACE};
	int length = sizeof(methodOption) / sizeof(methodOption[0]);
	char c = READ_UP_CHAR(b);

	for (int i = 0; i < length; i++) {
		if (methodOption[i][0] == c) {
			if (matchFormat(&(methodOption[i][1]), b)) {
				rData->method = methodType[i];
				break;
			}
		}
	}
	return rData->method == GET || rData->method == HEAD || rData->method == POST;
}

static bool checkUri (RequestData *rData, buffer *b) {
	bool isAbsolute = true;

	if (!matchFormat("HTTP", b)) {
		isAbsolute = false;
	}

	if (isAbsolute && PEEK_UP_CHAR(b) == 'S') {
		buffer_read(b);
	}

	if (isAbsolute && !matchFormat("://", b)) {
		isAbsolute = false;
	}

	if (!isAbsolute) {
		cleanRelativeUri(rData, b);
		return false;
	}

	return checkUriForHost(rData, b);
}

static void cleanRelativeUri (RequestData *rData, buffer *b) {
	char c;

	while ((c = PEEK_UP_CHAR(b)) != 0 && c != ' ' && c != '\t') {
		buffer_read(b);
	}
}

static bool checkUriForHost (RequestData *rData, buffer *b) {
	char c;
	int i = 0;

	// Si llego a este punto es porque tengu un uri absoluto que empieza con http:// o https://.

	while (i < HOST_MAX_SIZE && (c = READ_DOWN_CHAR(b)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else if (c == '@') { // Lo que copié hasta el momento era el userinfo.
			rData->host[0] = 0;
			i = 0;
		} else {
			break;
		}
	}
	return i < HOST_MAX_SIZE;
}

static bool extractHttpVersion (RequestData *rData, buffer *b) {
	char buf[VERSION_TEXT_SIZE + 1] = {0}; // Reserve space for NULL termination.
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

/**               FIN DE FUNCIONES DE START LINE                **/

/**               COMIENZO FUNCIONES DE HEADER                  **/

static bool checkHostHeader (RequestData *rData, buffer *b) {
	char c;
	bool hostHeader = false;

	while ((c = READ_UP_CHAR(b)) != 0) {
		if (c == 'H') {
			if (matchFormat("OST:", b)) {
				hostHeader = true;
				break;
			}
		} else if (c == '\r') {
			if (checkLF(b) && checkCRLF(b)) {
				// Busco CRLF consecutivos - fin headers
				break;
			}
		}
	}

	if (hostHeader) {
		return extractHost(rData, b);
	}

	return false;
}

static bool extractHost (RequestData *rData, buffer *b) {
	int i = 0;
	char c;

	moveThroughSpaces(b);

	while (i < HOST_MAX_SIZE && (c = READ_DOWN_CHAR(b)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else {
			break;
		}
	}
	return i < HOST_MAX_SIZE && (c == ' ' || c == '\t' || c == '\r');
}

/**               FIN DE FUNCIONES DE HEADER                    **/
