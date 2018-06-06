#include <requestParser/requestParser.h>

static bool
checkRequestInner (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
checkSpaces (RequestData *rData, buffer *bIn, buffer *bOut);
// Prototipos Start Line
static bool
extractHttpMethod (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
checkUri (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
cleanRelativeUri (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
checkUriForHost (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
extractHttpVersion (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
checkStartLineEnd (RequestData *rData, buffer *bIn, buffer *bOut);
// Prototipos Header
static bool
checkLocalHost (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
checkHostHeader (RequestData *rData, buffer *bIn, buffer *bOut);
static bool
extractHost (RequestData *rData, buffer *bIn, buffer *bOut);


void
defaultRequestStruct (RequestData *rData) {
	rData->parserState = METHOD;
	rData->isBufferEmpty = false;
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->method = UNDEFINED_M;
	rData->port = DEFAULT_PORT;
	rData->isLocalHost = false;

	for (int i = 0; i < HOST_MAX_SIZE; i++) {
		rData->host[i] = 0;
	}
}

bool
checkRequest (RequestData *rd, buffer *bIn, buffer *bOut,
			void(*hostCallback)(const char *, int, void*), void * callbackData) {
	bool success;

	// Reservo suficiente memoria para 1 RequestData struct.
	RequestData *rData=rd;

//	defaultRequestStruct(&rData);

	rData->hostCallback = hostCallback;
	rData->callbackData=callbackData;
    rData->version=V_1_1;//TODO fix groncho

	success = checkRequestInner(rData, bIn, bOut);

//	if (success == false) {
//		*state = (rData.state == OK ?
//			GENERAL_ERROR : rData.state);
//    if (success == false && !rData.isBufferEmpty) { //TODO: mfallone chekear
//        *state = (rData.state == OK ?
//                  GENERAL_ERROR : rData.state);
//	}

	return success;
}

static bool
checkRequestInner (RequestData *rData, buffer *bIn, buffer *bOut) {
	bool aux;
	bool success = true;
	bool active = true;


	rData->isBufferEmpty = false; // Necesario para transmisión intermitente de bytes.
	while (success && active) {
		switch (rData->parserState) {
			case SPACE_TRANSITION:
				if (!checkSpaces(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = rData->next;
				}
				break;
			case METHOD:
				if (!extractHttpMethod(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = SPACE_TRANSITION;
					rData->next = URI;
				}
				break;
			case URI:
				aux = checkUri(rData, bIn, bOut);
				// Si checkUri es falso pero no copié el host sigo adelante.
				// El problema lo tengo si copié host pero este es inválido.
				if (!aux && (rData->host[0] != 0 || rData->isBufferEmpty)) {
					success = false;
				} else if (aux) { // Ya encontré el host.
                    rData->hostCallback(rData->host, rData->port, rData->callbackData);

                    rData->parserState = FINISHED;
				} else {
					rData->parserState = SPACE_TRANSITION;
					rData->next = VERSION;
				}
				break;
			case URI_HOST:
				aux = checkUriForHost(rData, bIn, bOut);
				if (!aux && (rData->host[0] != 0 || rData->isBufferEmpty)) {
					success = false;
				} else if (aux) { // Ya encontré el host.
					rData->hostCallback(rData->host, rData->port, rData->callbackData);
					rData->parserState = FINISHED;
				} else {
					rData->parserState = SPACE_TRANSITION;
					rData->next = VERSION;
				}
				break;
			case RELATIVE_URI:
				if (!cleanRelativeUri(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = SPACE_TRANSITION;
					rData->next = VERSION;
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
				if (!checkStartLineEnd(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = LOCALHOST_HEADER_CHECK;
				}
				break;
			case LOCALHOST_HEADER_CHECK:
				if (checkLocalHost(rData, bIn, bOut)) { // Ya encontré el host.
					rData->hostCallback(rData->host, rData->port, rData->callbackData);
					rData->parserState = FINISHED;
				} else if (rData->isBufferEmpty) {
					success = false;
					break;
				} else {
					rData->parserState = HEADERS;
				}
				break;
			case HEADERS:
				if (checkHostHeader(rData, bIn, bOut)) { // Ya encontré el host header.
					rData->parserState = SPACE_TRANSITION;
					rData->next = HOST;
				} else {
					success = false;
				}
				break;
			case HOST:
				aux = extractHost(rData, bIn, bOut);
				if (!aux && (rData->host[0] != 0 || rData->isBufferEmpty)) {
					success = false;
				} else if (aux) { // Ya encontré el host.
					rData->hostCallback(rData->host, rData->port, rData->callbackData);
					rData->parserState = FINISHED;
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
			case SPACE_TRANSITION:
				break;
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
			case HEADERS:
				rData->state = HOST_ERROR;
				break;
			case HOST:
				rData->state = HOST_ERROR;
				break;
			case FINISHED:
				//readAndWrite(bIn,bOut);
				break;
		}
	}

	return success;
}

static bool
checkSpaces (RequestData *rData, buffer *bIn, buffer *bOut) {
	if (moveThroughSpaces(bIn) == 0) {
		rData->isBufferEmpty = true;
		return false;
	}
	buffer_write(bOut, ' ');
	return true;
}

/**               COMIENZO FUNCIONES DE START LINE              **/

static bool
extractHttpMethod (RequestData *rData, buffer *bIn, buffer *bOut) {
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
			if (matchFormat(&(methodOption[i][1]), bIn, bOut, methodPrefix, &(rData->isBufferEmpty))) {
				rData->method = methodType[i];
				break;
			} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
				break;
			}
		}
	}

	return rData->method == GET || rData->method == HEAD || rData->method == POST;
}

static bool
checkUri (RequestData *rData, buffer *bIn, buffer *bOut) {
	if (matchFormat("HTTP", bIn, bOut, "", &(rData->isBufferEmpty))) {
		if (matchFormat("://", bIn, bOut, "HTTP", &(rData->isBufferEmpty))) {
			rData->parserState = URI_HOST;

			if (checkUriForHost(rData, bIn, bOut)) {
				return true;
			} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
				return false;
			}
		} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
			return false;
		}
	} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
		return false;
	}

	rData->parserState = RELATIVE_URI;

	cleanRelativeUri(rData, bIn, bOut);

	return false;
}

static bool
cleanRelativeUri (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c;

	// El relative uri no es de interés para el parser por lo que solo busco pasarlo de largo.
	while ((c = buffer_peek(bIn)) != 0 && c != ' ' && c != '\t') {
		readAndWrite(bIn, bOut);
	};

	if (c == 0)  {
		rData->isBufferEmpty = true;
		return false;
	}
	return true;
}

static bool
checkUriForHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	int i = 0;
	char c;
	bool validHost = true;

	// Si llego a este punto es porque tengu un uri absoluto que empieza con http:// o https://.

	while (i < HOST_MAX_SIZE && (c = PEEK_DOWN_CHAR(bIn)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else if (c == '@') { // Lo que copié hasta el momento era el userinfo.
			rData->host[0] = 0;
			i = 0;
		} else {
			break;
		}
		readAndWrite(bIn, bOut);
	}

	// En principio, no soporto IPv6.
	if (c == ':')  {
		readAndWrite(bIn, bOut);
		validHost = getNumber(&(rData->port), bIn, bOut, ":", &(rData->isBufferEmpty));
	}

	if (c == 0 || (!validHost && rData->isBufferEmpty))  {
		writeToBufReverse(rData->host, bIn, i);
		rData->isBufferEmpty = true; // Necesario para el caso en que c == 0.
		return false;
	}

	return i < HOST_MAX_SIZE && validHost;
}

static bool
extractHttpVersion (RequestData *rData, buffer *bIn, buffer *bOut) {
	char versionOption[] = {'0', '1'};
	httpVersion versionType[] = {V_1_0, V_1_1};
	int length = sizeof(versionType) / sizeof(versionType[0]);
	char c;

	if (matchFormat("HTTP/1.", bIn, bOut, "", &(rData->isBufferEmpty))) {
		if ((c = readAndWrite(bIn, bOut)) != 0) {
			for (int i = 0; i < length; i++) {
				if (versionOption[i] == c) {
					rData->version = versionType[i];
					break;
				}
			}
		} else {
			rData->isBufferEmpty = true;
			writePrefix(bIn, "HTTP/1.");
			return false;
		}
	} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
		return false;
	}

	return rData->version != UNDEFINED;
}

static bool
checkStartLineEnd (RequestData *rData, buffer *bIn, buffer *bOut) {
	return checkCRLF(bIn, bOut, "", &(rData->isBufferEmpty));
}

/**               FIN DE FUNCIONES DE START LINE                **/

/**               COMIENZO FUNCIONES DE HEADER                  **/

static bool
checkLocalHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c = PEEK_UP_CHAR(bIn);

	if (c == 0) {
		rData->isBufferEmpty = true;
		return false;
	}
	// Si después de la start line viene un header LOOP, tengo un loop habiendo
	// puesto dicho header artificial en una llamada anterior a checkLocalHost.
	if (c != 'X') {
		writeToBuf("X-LOCALHOST: TRUE\r\n", bOut);
	} else if (matchFormat("X-LOCALHOST:", bIn, bOut, "", &(rData->isBufferEmpty))) {
		rData->isLocalHost = true;
	} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
		return false;
	}
	return rData->isLocalHost;
}

static bool
checkHostHeader (RequestData *rData, buffer *bIn, buffer *bOut) {
	char c;
	bool hostHeader = false;
	bool headersEnd = false;
	while ((c = READ_UP_CHAR(bIn, bOut)) != 0) {
		if (c == 'H') {
			if (matchFormat("OST:", bIn, bOut, "H", &(rData->isBufferEmpty))) {
				hostHeader = true;
				break;
			} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
				break;
			}
		} else if (c == '\r') {
			if (checkLF(bIn, bOut, "\r", &(rData->isBufferEmpty))
			  && checkCRLF(bIn, bOut, "\r\n", &(rData->isBufferEmpty))) { // Busco el CRLF CRLF.
				headersEnd = true;
				break;
			} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
				break;
			}
		}
	}

	if (hostHeader) {
		return true;
	}
	if (headersEnd) {
		return false; // No encontré el header host.
	}

	rData->isBufferEmpty = true; // Necesario para el caso en que c == 0.
	return false;
}

static bool
extractHost (RequestData *rData, buffer *bIn, buffer *bOut) {
	int i = 0;
	char c;
	bool validHost = true;

	while (i < HOST_MAX_SIZE && (c = PEEK_DOWN_CHAR(bIn)) != 0) {
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.') {
			rData->host[i++] = c;
		} else {
			break;
		}
		readAndWrite(bIn, bOut);
	}

	// En principio, no soporto IPv6.
	if (c == ':')  {
		readAndWrite(bIn, bOut);
		validHost = getNumber(&(rData->port), bIn, bOut, ":", &(rData->isBufferEmpty));
	}

	if (c == 0 || (!validHost && rData->isBufferEmpty))  {
		writeToBufReverse(rData->host, bIn, i);
		rData->isBufferEmpty = true; // Necesario para el caso en que c == 0.
		return false;
	}

	c = buffer_peek(bIn); // Por las validaciones anteriores ya se que voy a tener algo != 0.

	return i < HOST_MAX_SIZE && validHost && (c == ' ' || c == '\t' || c == '\r');
}

/**               FIN DE FUNCIONES DE HEADER                    **/
