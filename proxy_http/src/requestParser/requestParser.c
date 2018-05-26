#include <requestParser/requestParser.h>

static bool checkRequestInner (RequestData *rData, buffer *bIn, buffer *bOut);
// Prototipos Start Line
static bool extractHttpMethod (RequestData *rData, buffer *bIn, buffer *bOut);
static bool checkUri (RequestData *rData, buffer *bIn, buffer *bOut);
static bool cleanRelativeUri (RequestData *rData, buffer *bIn, buffer *bOut);
static bool checkUriForHost (RequestData *rData, buffer *bIn, buffer *bOut);
static bool extractHttpVersion (RequestData *rData, buffer *bIn, buffer *bOut);
// Prototipos Header
static bool checkLocalHost (RequestData *rData, buffer *bIn, buffer *bOut);
static bool checkHostHeader (RequestData *rData, buffer *bIn, buffer *bOut);
static bool extractHost (RequestData *rData, buffer *bIn, buffer *bOut);


void defaultRequestStruct (RequestData *rData) {
	rData->parserState = METHOD;
	rData->isBufferEmpty = false;
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->method = UNDEFINED_M;
	rData->isLocalHost = false;

	for (int i = 0; i < HOST_MAX_SIZE; i++) {
		rData->host[i] = 0;
	}
}

bool checkRequest (requestState *state, buffer *bIn, buffer *bOut) {
	bool success;

	// Reservo suficiente memoria para 1 RequestData struct.
	RequestData *rData = (RequestData *) malloc(sizeof(RequestData));

	if (rData == NULL) {
		*state = ALLOCATION_ERROR;
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	defaultRequestStruct(rData);

	success = checkRequestInner(rData, bIn, bOut);

	if (success == false) {
		*state = (rData->state == OK ?
			GENERAL_ERROR : rData->state);
	}

	free(rData);

	return success;
}

static bool checkRequestInner (RequestData *rData, buffer *bIn, buffer *bOut) {
	bool aux;
	bool success = true;
	bool active = true;
	
	rData->isBufferEmpty = false; // Necesario para transmisión intermitente de bytes.

	while (success && active) {
		switch (rData->parserState) {
			case METHOD:
				if (!extractHttpMethod(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = URI;
				}
				break;
			case URI:
				aux = checkUri(rData, bIn, bOut);
				// Si checkUri es falso pero no copié el host sigo adelante.
				// El problema lo tengo si copié host pero este es inválido.
				if (!aux && (rData->host[0] != 0 || rData->isBufferEmpty)) {
					success = false;
				} else if (aux) { // Ya encontré el host.
					rData->parserState = FINISHED;
				} else {
					rData->parserState = VERSION;
				}
				break;
			case URI_HOST:
				aux = checkUriForHost(rData, bIn, bOut);
				if (!aux && (rData->host[0] != 0 || rData->isBufferEmpty)) {
					success = false;
				} else if (aux) { // Ya encontré el host.
					rData->parserState = FINISHED;
				} else {
					rData->parserState = VERSION;
				}
				break;
			case RELATIVE_URI:
				if (!cleanRelativeUri(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = VERSION;
				}
				break;
			case VERSION:
				if (!extractHttpVersion(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = START_LINE_END;
				}
				break;
			case START_LINE_END:
				if (!checkCRLF(bIn, bOut, "")) {
					success = false;
				} else {
					rData->parserState = LOCALHOST_HEADER_CHECK;
				}
				break;
			case LOCALHOST_HEADER_CHECK:
				if (checkLocalHost(rData, bIn, bOut)) { // Ya encontré el host.
					rData->parserState = FINISHED;
				} else {
					rData->parserState = HOST_HEADER_CHECK;
				}
				break;
			case HOST_HEADER_CHECK:
				if (checkHostHeader(rData, bIn, bOut)) { // Ya encontré el host.
					rData->parserState = FINISHED;
				} else {
					success = false;
				}
				break;
			case FINISHED:
					active = false;
				break;
		}
	}

	if (!success && !rData->isBufferEmpty) {
		// Si rData->isBufferEmpty == true rompí porque no había para leer en el buffer.
		// Sino es porque hubo un error el request.
		switch (rData->parserState) {
			case METHOD:
				rData->state = (rData->method == UNDEFINED_M ? 
						GENERAL_METHOD_ERROR : UNSUPPORTED_METHOD_ERROR);
				break;
			case URI:
				rData->state = HOST_ERROR;
				break;
			case URI_HOST:
				rData->state = HOST_ERROR;
				break;
			case RELATIVE_URI:
				break;
			case VERSION:
				rData->state = VERSION_ERROR;
				break;
			case START_LINE_END:
				rData->state = START_LINE_END_ERROR;
				break;
			case LOCALHOST_HEADER_CHECK:
				break;
			case HOST_HEADER_CHECK:
				rData->state = HOST_ERROR;
				break;
			case FINISHED:
				break;
		}
	}

	return success;
}

/**               COMIENZO FUNCIONES DE START LINE              **/

static bool extractHttpMethod (RequestData *rData, buffer *bIn, buffer *bOut) {
	char *methodOption[] = {"CONNECT", "DELETE", "GET", "HEAD", "OPTIONS", "POST", "PUT", "TRACE"};
	httpMethod methodType[] = {CONNECT, DELETE, GET, HEAD, OPTIONS, POST, PUT, TRACE};
	int length = sizeof(methodOption) / sizeof(methodOption[0]);
	char c = READ_UP_CHAR(bIn, bOut);
	char methodPrefix[2] = {c, 0};

	if (c == 0) {
		rData->isBufferEmpty = true;
		return false;
	}

	for (int i = 0; i < length; i++) {
		if (methodOption[i][0] == c) {
			if (matchFormat(&(methodOption[i][1]), bIn, bOut, methodPrefix)) {
				rData->method = methodType[i];
				break;
			} else if (buffer_peek(bIn) == 0) {
				rData->isBufferEmpty = true;
				break;
			}
		}
	}

	moveThroughSpaces(bIn, bOut);

	return rData->method == GET || rData->method == HEAD || rData->method == POST;
}

static bool checkUri (RequestData *rData, buffer *bIn, buffer *bOut) {
	bool isAbsolute = true;

	if (!matchFormat("HTTP", bIn, bOut, "")) {
		isAbsolute = false;

		if (buffer_peek(bIn) == 0)  {
			rData->isBufferEmpty = true;
			return false;
		}
	}

	if (isAbsolute && PEEK_UP_CHAR(bIn) == 'S') {
		readAndWrite(bIn, bOut);
	}

	if (isAbsolute && !matchFormat("://", bIn, bOut, "HTTP")) {
		isAbsolute = false;

		if (buffer_peek(bIn) == 0)  {
			rData->isBufferEmpty = true;
			return false;
		}
	}

	if (!isAbsolute) {
		cleanRelativeUri(rData, bIn, bOut);
		return false;
	}

	return checkUriForHost(rData, bIn, bOut);
}

static bool cleanRelativeUri (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c;

	while ((c = PEEK_UP_CHAR(bIn)) != 0 && c != ' ' && c != '\t') {
		buffer_read(bIn);
	}

	if (c == 0)  {
		rData->isBufferEmpty = true;
		return false;
	}

	moveThroughSpaces(bIn, bOut);

	return true;
}

static bool checkUriForHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c;
	int i = 0;

	// Si llego a este punto es porque tengu un uri absoluto que empieza con http:// o https://.

	while (i < HOST_MAX_SIZE && (c = READ_DOWN_CHAR(bIn, bOut)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else if (c == '@') { // Lo que copié hasta el momento era el userinfo.
			rData->host[0] = 0;
			i = 0;
		} else {
			break;
		}
	}

	if (c == 0)  {
		while (i > 0) {
			i--;
			buffer_write_reserved(bIn, rData->host[i]);
		}
		rData->isBufferEmpty = true;
		return false;
	}

	moveThroughSpaces(bIn, bOut);

	return i < HOST_MAX_SIZE;
}

static bool extractHttpVersion (RequestData *rData, buffer *bIn, buffer *bOut) {
	char *versionOption[] = {"1.0", "1.1"};
	httpVersion versionType[] = {V_1_0, V_1_1};
	int length = sizeof(versionType) / sizeof(versionType[0]);
	char c;

	if (!matchFormat("HTTP/", bIn, bOut, "")) {
		rData->state = START_LINE_FORMAT_ERROR;
		return false;
	}

	if (matchFormat("1.", bIn, bOut, "HTTP/")) {
		c = READ_UP_CHAR(bIn, bOut);

		for (int i = 0; i < length; i++) {
			if (versionOption[i][2] == c) {
				rData->version = versionType[i];
				break;
			}
		}
	}

	if (rData->version == UNDEFINED && buffer_peek(bIn) == 0)  {
		rData->isBufferEmpty = true;
		return false;
	}

	moveThroughSpaces(bIn, bOut);

	return rData->version != UNDEFINED;
}

/**               FIN DE FUNCIONES DE START LINE                **/

/**               COMIENZO FUNCIONES DE HEADER                  **/

static bool checkLocalHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	if (buffer_peek(b) != 'L') {
		writeToBuf ("LOCALHOST: TRUE\r\n", bOut);
	} else {
		if (matchFormat("LOCALHOST:", bIn, bOut, "")) {
			rData->isLocalHost = true;
		}
	}

	if (rData->isLocalHost == false && buffer_peek(bIn) == 0)  {
		rData->isBufferEmpty = true;
		return false;
	}

	return rData->isLocalHost;
}

static bool checkHostHeader (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c;
	bool hostHeader = false;

	while ((c = READ_UP_CHAR(bIn, bOut)) != 0) {
		if (c == 'H') {
			if (matchFormat("OST:", bIn, bOut, "H")) {
				hostHeader = true;
				break;
			}
		} else if (c == '\r') {
			if (checkLF(bIn, bOut, "\r") && checkCRLF(bIn, bOut, "\r\n")) {
				// Busco CRLF consecutivos - fin headers
				break;
			}
		}
	}

	if (hostHeader) {
		return extractHost(rData, bIn, bOut);
	}

	if (c == 0)  {
		rData->isBufferEmpty = true;
	}

	return false;
}

static bool extractHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	int i = 0;
	char c;

	moveThroughSpaces(bIn, bOut);

	while (i < HOST_MAX_SIZE && (c = READ_DOWN_CHAR(bIn, bOut)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else {
			break;
		}
	}

	if (c == 0)  {
		while (i > 0) {
			i--;
			buffer_write_reserved(bIn, rData->host[i]);
		}
		rData->isBufferEmpty = true;
		return false;
	}

	// Recordar que el header host puede incluir un puerto.
	return i < HOST_MAX_SIZE && (c == ' ' || c == '\t' || c == '\r' || c == ':');
}

/**               FIN DE FUNCIONES DE HEADER                    **/
