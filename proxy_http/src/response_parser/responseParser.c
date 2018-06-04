#include "responseParser.h"

static bool
checkResponseInner (ResponseData *rData, buffer *bIn, buffer *bOut, buffer *bTransf);
static bool
checkSpaces (ResponseData *rData, buffer *bIn, buffer *bOut);
// Prototipos Start Line
static bool
extractHttpVersion (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
extractStatus (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
isValidStatus (const int status);
// Prototipos Header
static bool
checkHeaders (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
checkLength (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
checkChunked (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
checkType (ResponseData *rData, buffer *bIn, buffer *bOut);
// Prototipos Body
static bool
extractBody (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
extractChunkedBody (ResponseData *rData, buffer *bIn, buffer *bOut);
static bool
extractBodyTransf (ResponseData *rData, buffer *bIn, buffer *bTransf);
static bool
extractChunkedBodyTransf (ResponseData *rData, buffer *bIn, buffer *bTransf);


void
defaultResponseStruct (ResponseData *rData) {
	rData->parserState = VERSION;
	rData->isBufferEmpty = false;
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->status = 0;
	rData->bodyLength = NO_BODY_LENGTH;
	rData->isChunked = false;
}

bool
checkResponse (responseState *state, buffer *bIn, buffer *bOut, buffer *bTransf) {
	bool success;

	// Reservo suficiente memoria para 1 ResponseData struct.
	ResponseData *rData = (ResponseData *) malloc(sizeof(ResponseData));

	if (rData == NULL) {
		rData->state = ALLOCATION_ERROR;
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	defaultResponseStruct(rData);

	success = checkResponseInner(rData, bIn, bOut, bTransf);

	if (success == false) {
		*state = (rData->state == OK ?
			GENERAL_ERROR : rData->state);
	}

	free(rData);

	return success;
}

static bool
checkResponseInner (ResponseData *rData, buffer *bIn, buffer *bOut, buffer *bTransf) {
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
			case VERSION:
				if (!extractHttpVersion(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = SPACE_TRANSITION;
					rData->next = STATUS;
				}
				break;
			case STATUS:
				if (!extractStatus(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = HEADERS;
				}
				break;
			case HEADERS:
				if (!checkHeaders(rData, bIn, bOut)) {
					success = false;
				} else {
					if (rData->next == FINISHED) { // Si encontré end of headers.
						if (rData->withTransf) {
							rData->parserState = BODY_TRANSFORMATION;
						} else {
							rData->parserState = BODY_NORMAL;
						}
					} else {
						rData->parserState = SPACE_TRANSITION;
					}
					// El next ya lo marqué dentro de checkHeaders.
				}
				break;
			case LENGTH_CHECK:
				if (!checkLength(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = HEADERS;
				}
				break;
			case ENCODING_CHECK:
				if (!checkChunked(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = HEADERS;
				}
				break;
			case TYPE_CHECK:
				if (!checkType(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = HEADERS;
				}
				break;
			case BODY_NORMAL:
				if (!extractBody(rData, bIn, bOut)) {
					success = false;
				} else {
					rData->parserState = FINISHED;
				}
				break;
			case BODY_TRANSFORMATION:
				if (!extractBodyTransf(rData, bIn, bTransf)) {
					success = false;
				} else {
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
			case VERSION:
				rData->state = VERSION_ERROR;
				break;
			case STATUS:
				rData->state = STATUS_ERROR;
				break;
			case HEADERS:
				break;
			case LENGTH_CHECK:
				break;
			case ENCODING_CHECK:
				break;
			case TYPE_CHECK:
				break;
			case BODY_NORMAL:
				if (rData->isChunked) {
					rData->state = CHUNKS_ERROR;
				}
				break;
			case BODY_TRANSFORMATION:
				if (rData->isChunked) {
					rData->state = CHUNKS_ERROR;
				}
				break;
			case FINISHED:
				break;
		}
	}

	return success;
}

static bool
checkSpaces (ResponseData *rData, buffer *bIn, buffer *bOut) {
	if (moveThroughSpaces(bIn) == 0) {
		rData->isBufferEmpty = true;
		return false;
	}
	buffer_write(bOut, ' ');
	return true;
}

/**               COMIENZO FUNCIONES DE START LINE              **/

static bool extractHttpVersion
(ResponseData *rData, buffer *bIn, buffer *bOut) {
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
extractStatus (ResponseData *rData, buffer *bIn, buffer *bOut) {
	if (getNumber(&(rData->status), bIn, bOut, "", &(rData->isBufferEmpty))) {
		if (isValidStatus(rData->status)) {
			return true;
		}
	}

	return false; // Puede que salga porque el buffer está vacío o porque encontré algo distinto de un número.
}

static bool
isValidStatus (const int status) {
	return status == STATUS_OK;
}

/**               FIN DE FUNCIONES DE START LINE                **/

/**               COMIENZO FUNCIONES DE HEADER                  **/

static bool
checkHeaders (ResponseData *rData, buffer *bIn, buffer *bOut) {
	char aux, c;
	bool lengthHeader = false;
	bool transferHeader = false;
	bool typeHeader = false;
	bool headersEnd = false;

	while ((c = READ_UP_CHAR(bIn, bOut)) != 0) {
		if (c == 'C') {
			if (matchFormat("ONTENT-", bIn, bOut, "C", &(rData->isBufferEmpty))) {
				if ((aux = READ_UP_CHAR(bIn, bOut)) == 'L') {
					if (matchFormat("ENGTH:", bIn, bOut, "CONTENT-L", &(rData->isBufferEmpty))) {
						lengthHeader = true;
						break;
					} else if (rData->isBufferEmpty) {
						break;
					}
				} else if (aux == 'E') {
					if (matchFormat("NCODING:", bIn, bOut, "CONTENT-E", &(rData->isBufferEmpty))) {
						transferHeader = true;
						break;
					} else if (rData->isBufferEmpty) {
						break;
					}
				} else if (aux == 'T') {
					if (matchFormat("YPE:", bIn, bOut, "CONTENT-T", &(rData->isBufferEmpty))) {
						typeHeader = true;
						break;
					} else if (rData->isBufferEmpty) {
						break;
					}
				} else if (aux == 0) {
					rData->isBufferEmpty = true;
					writePrefix(bIn, "CONTENT-");
					break;
				}
			} else if (rData->isBufferEmpty) { // En algún momento el buffer quedó vacío.
				break;
			}
		} else if (c == 'T') {
			if (matchFormat("RANSFER-ENCODING:", bIn, bOut, "T", &(rData->isBufferEmpty))) {
				transferHeader = true;
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

	if (lengthHeader) {
		rData->next = LENGTH_CHECK;
		return true;
	}
	if (transferHeader) {
		rData->next = ENCODING_CHECK;
		return true;
	}
	if (typeHeader) {
		rData->next = TYPE_CHECK;
		return true;
	}
	if (headersEnd) {
		rData->next = FINISHED;
		return true;
	}

	rData->isBufferEmpty = true; // Necesario para el caso en que c == 0.
	return false;
}

static bool
checkLength (ResponseData *rData, buffer *bIn, buffer *bOut) {
	return getNumber(&(rData->bodyLength), bIn, bOut, "", &(rData->isBufferEmpty));
}

static bool
checkChunked (ResponseData *rData, buffer *bIn, buffer *bOut) {
	if (matchFormat("CHUNKED", bIn, bOut, "", &(rData->isBufferEmpty))) {
		rData->isChunked = true;
		return true;
	}
	return false;
}

// A implementar
static bool
checkType (ResponseData *rData, buffer *bIn, buffer *bOut) {
	return true;
}

/**               FIN DE FUNCIONES DE HEADER                    **/

/**               COMIENZO FUNCIONES DE BODY                    **/

static bool
extractBody (ResponseData *rData, buffer *bIn, buffer *bOut) {
	// Chunked encoding sobreescribe content length.
	if (rData->isChunked) {
		return extractChunkedBody(rData, bIn, bOut);
	}
	if (rData->bodyLength > 0) {
		if (!writeToTransfBuf(bIn, bOut, &(rData->bodyLength))) {
			if (rData->bodyLength > 0) {
				rData->isBufferEmpty = true;
			}
			return false;
		}
	}
	return true;
}

static bool
extractChunkedBody (ResponseData *rData, buffer *bIn, buffer *bOut) {
	int chunkLength = 0;

	// Con el do while, la última iteración corresponde a dos
	// empty line consecutivos como corresponde.
	do {
		if (!getHexNumber(&(rData->bodyLength), bIn, bOut, "", &(rData->isBufferEmpty))) {
			return false;
		}
		chunkLength = rData->bodyLength;

		if (!checkCRLF(bIn, bOut, "", &(rData->isBufferEmpty))) { // Delimitador de longitud de chunk.
			if (rData->isBufferEmpty) {
				writeHexToBufReverse(rData->bodyLength, bIn); // Vuelvo a poner el chunk length.
			}
			return false;
		}

		if (!writeToTransfBuf(bIn, bOut, &(rData->bodyLength))) {
			// si lectura se corta, en bodyLength me queda lo que me faltaba por leer.
			if (rData->bodyLength > 0) {
				rData->isBufferEmpty = true;
				writePrefix(bIn, "\r\n");
				writeHexToBufReverse(rData->bodyLength, bIn); // Vuelvo a poner el chunk length.
			}
			return false;
		}

		if (!checkCRLF(bIn, bOut, "", &(rData->isBufferEmpty))) { // Delimitador de chunk.
			// Si buffer queda vacío, escribo un chunk cualquiera para que la siguiente vez que lea
			// no parezca como que tengo un fin de chunks. Al escribir en zona reservada, el chuck
			// ficticio no va al buffer de salida en la siguiente pasada como es de esperar.
			// Si hay un fin de chunks pongo lo que tiene que ir para el mismo.
			writePrefix(bIn, chunkLength > 0 ? "1\r\n1" : "0\r\n");
			return false;
		}
	} while (chunkLength > 0);

	return true;
}

static bool
extractBodyTransf (ResponseData *rData, buffer *bIn, buffer *bTransf) {
	// Chunked encoding sobreescribe content length.
	if (rData->isChunked) {
		return extractChunkedBodyTransf(rData, bIn, bTransf);
	}
	if (rData->bodyLength > 0) {
		if (!writeToTransfBuf(bIn, bTransf, &(rData->bodyLength))) {
			if (rData->bodyLength > 0) {
				rData->isBufferEmpty = true;
			}
			return false;
		}
	}
	return true;
}

static bool
extractChunkedBodyTransf (ResponseData *rData, buffer *bIn, buffer *bTransf) {
	int chunkLength = 0;

	// Con el do while, la última iteración corresponde a dos
	// empty line consecutivos como corresponde.
	do {
		if (!simpleGetHexNumber(&(rData->bodyLength), bIn, "", &(rData->isBufferEmpty))) {
			return false;
		}
		chunkLength = rData->bodyLength;

		if (!simpleCheckCRLF(bIn, "", &(rData->isBufferEmpty))) { // Delimitador de longitud de chunk.
			if (rData->isBufferEmpty) {
				writeHexToBufReverse(rData->bodyLength, bIn); // Vuelvo a poner el chunk length.
			}
			return false;
		}

		if (!writeToTransfBuf(bIn, bTransf, &(rData->bodyLength))) {
			// si lectura se corta, en bodyLength me queda lo que me faltaba por leer.
			if (rData->bodyLength > 0) {
				rData->isBufferEmpty = true;
				writePrefix(bIn, "\r\n");
				writeHexToBufReverse(rData->bodyLength, bIn); // Vuelvo a poner el chunk length.
			}
			return false;
		}

		if (!simpleCheckCRLF(bIn, "", &(rData->isBufferEmpty))) { // Delimitador de chunk.
			// Si buffer queda vacío, escribo un chunk cualquiera para que la siguiente vez que lea
			// no parezca como que tengo un fin de chunks. Al escribir en zona reservada, el chuck
			// ficticio no va al buffer de salida en la siguiente pasada como es de esperar.
			// Si hay un fin de chunks pongo lo que tiene que ir para el mismo.
			writePrefix(bIn, chunkLength > 0 ? "1\r\n1" : "0\r\n");
			return false;
		}
	} while (chunkLength > 0);

	return true;
}

/**               FIN DE FUNCIONES DE BODY                       **/
