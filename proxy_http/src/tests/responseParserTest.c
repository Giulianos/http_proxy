#include "responseParser/responseParser.c" // Source code included to test static functions.
#include <assert.h>

static void
assertVersion (ResponseData *rData, buffer *b, buffer *bOut);
static void
assertStatus (ResponseData *rData, buffer *b, buffer *bOut);
static void
assertCompleteResponseWithLength (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertCompleteResponseWithLengthAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertCompleteResponseWithChunk (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertCompleteResponseWithChunkAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertIncompleteResponseWithChunk (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertIncompleteResponseWithChunkAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertIncompleteResponseWithLengthByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertIncompleteResponseWithChunkByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
assertIncompleteResponseWithChunkAndTransfByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf);
static void
insertToBuffer (ResponseData *rData, char *text, buffer *b, buffer *bOut);
static void
resetData (ResponseData *rData, buffer *b, buffer *bOut);

int
main (int argc, char *argv[]) {
	ResponseData *rData = (ResponseData *) malloc(sizeof(ResponseData));
	struct buffer b;
	uint8_t direct_buff[100];
	int totalSpace = 100;
	int reservedSpace = 20;
	struct buffer bOut;
	uint8_t direct_buff_out[100];
	int totalSpaceOut = 100;
	struct buffer bTransf;
	uint8_t direct_buff_transf[100];
	int totalSpaceTransf = 100;
	buffer_init_r(&b, reservedSpace, totalSpace, direct_buff);
	buffer_init(&bOut, totalSpaceOut, direct_buff_out);
	buffer_init(&bTransf, totalSpaceTransf, direct_buff_transf);


	if (rData == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	assertVersion(rData, &b, &bOut);
	assertStatus(rData, &b, &bOut);
	assertCompleteResponseWithLength(rData, &b, &bOut, &bTransf);
	assertCompleteResponseWithLengthAndTransf(rData, &b, &bOut, &bTransf);
	assertCompleteResponseWithChunk(rData, &b, &bOut, &bTransf);
	assertCompleteResponseWithChunkAndTransf(rData, &b, &bOut, &bTransf);
	assertIncompleteResponseWithChunk(rData, &b, &bOut, &bTransf);
	assertIncompleteResponseWithChunkAndTransf(rData, &b, &bOut, &bTransf);
	assertIncompleteResponseWithLengthByByte(rData, &b, &bOut, &bTransf);
	assertIncompleteResponseWithChunkByByte(rData, &b, &bOut, &bTransf);
	assertIncompleteResponseWithChunkAndTransfByByte(rData, &b, &bOut, &bTransf);

	free(rData);

	return 0;
}

static void
assertVersion (ResponseData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "hTt", b, bOut);
	assert(!extractHttpVersion(rData, b, bOut));
	writeToBuf("p/1.", b);
	assert(!extractHttpVersion(rData, b, bOut));
	buffer_write(b, '1');
	assert(rData->version == UNDEFINED);
	assert(extractHttpVersion(rData, b, bOut));
	assert(rData->version == V_1_1);

	insertToBuffer(rData, "hTtp/1.2", b, bOut); // Versión incorrecta.
	assert(!extractHttpVersion(rData, b, bOut));
	assert(rData->version == UNDEFINED);
}

static void
assertStatus (ResponseData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "d200", b, bOut);
	assert(!extractStatus(rData, b, bOut));
	buffer_read(b); // Saco la d.
	assert(!extractStatus(rData, b, bOut)); // No se si tengo otro número después del 0.
	buffer_write(b, ' ');
	assert(extractStatus(rData, b, bOut));
	assert(rData->status == 200);

	insertToBuffer(rData, "400 ", b, bOut);
	assert(!extractStatus(rData, b, bOut));
	assert(rData->status == 400);
}

static void
assertCompleteResponseWithLength (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWikipedia", b, bOut);
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->bodyLength == 0); // Terminé de leer todo.
	assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertCompleteResponseWithLengthAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWikipedia", b, bOut);
	rData->withTransf = true;
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->bodyLength == 0); // Terminé de leer todo.
	assert(buffer_read(bTransf) == 'W');
	assert(buffer_read(bTransf) == 'i');
	assert(buffer_read(bTransf) == 'k');
	assert(buffer_read(bTransf) == 0);
}

static void
assertCompleteResponseWithChunk (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTent-encoDing:chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n", b, bOut);
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->isChunked);
	assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertCompleteResponseWithChunkAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	char *msg = "Wikipedia";
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTent-encoDing:chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n", b, bOut);
	rData->withTransf = true;
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->isChunked);

	// Al buffer de la transformación espero haber paso solo el contenido de los chunks.
	for (int i = 0; msg[i] != 0; i++) {
		assert(buffer_read(bTransf) == msg[i]);
	}
	assert(buffer_read(bTransf) == 0);
}

static void
assertIncompleteResponseWithChunk (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	insertToBuffer(rData, "HTTP/1.", b, bOut);
	assert(rData->parserState == VERSION);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	writeToBuf("1 20", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->version == V_1_1);
	assert(rData->parserState == STATUS);
	writeToBuf("0 OK\r\nconTent-enc", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->status == 200);
	assert(rData->parserState == HEADERS);
	writeToBuf("oDing:c", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == ENCODING_CHECK);
	writeToBuf("hunkEd\r\n\r\n", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->isChunked);
	assert(rData->parserState == BODY_NORMAL);
	writeToBuf("4\r\nWi", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	writeToBuf("ki\r\n5\r\npedia\r\n0\r\n\r\n", b);
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);
}

static void
assertIncompleteResponseWithChunkAndTransf (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	char *msg = "Wikipedia";
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTent-encoDing:chunkEd\r\n\r\n", b, bOut);
	rData->withTransf = true;
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->isChunked);
	assert(rData->parserState == BODY_TRANSFORMATION);
	writeToBuf("4\r\nWi", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	writeToBuf("ki\r\n5\r\npedia\r", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	writeToBuf("\n0\r\n\r", b);
	assert(!checkResponseInner(rData, b, bOut, bTransf));
	writeToBuf("\n", b);
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);

	// Al buffer de la transformación espero haber paso solo el contenido de los chunks.
	for (int i = 0; msg[i] != 0; i++) {
		assert(buffer_read(bTransf) == msg[i]);
	}
	assert(buffer_read(bTransf) == 0);
}

static void
assertIncompleteResponseWithLengthByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	char *msg = "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWik";
	char aux[2] = {0};
	insertToBuffer(rData, "", b, bOut);
	int i = 0;
	while (msg[i] != 0) {
		aux[0] = msg[i];
		// Hago la comparación del paso anterior. Esto lo hago para que no rompa en el último paso.
		assert(!checkResponseInner(rData, b, bOut, bTransf));
		writeToBuf(aux, b);
		i++;
	}
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->bodyLength == 0); // Terminé de leer todo.
	assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertIncompleteResponseWithChunkByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	char *msg = "HTTP/1.1 200 OK\r\nconTent-encoDing:chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n";
	char aux[2] = {0};
	insertToBuffer(rData, "", b, bOut);
	int i = 0;
	while (msg[i] != 0) {
		aux[0] = msg[i];
		// Hago la comparación del paso anterior. Esto lo hago para que no rompa en el último paso.
		assert(!checkResponseInner(rData, b, bOut, bTransf));
		writeToBuf(aux, b);
		i++;
	}
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);
	assert(rData->version == V_1_1);
	assert(rData->status == 200);
	assert(rData->isChunked);
	assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertIncompleteResponseWithChunkAndTransfByByte (ResponseData *rData, buffer *b, buffer *bOut, buffer *bTransf) {
	char *msgTransf = "Wikipedia";
	char *msg = "HTTP/1.1 200 OK\r\nconTent-encoDing:chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n";
	char aux[2] = {0};
	insertToBuffer(rData, "", b, bOut);
	rData->withTransf = true;
	int i = 0;
	while (msg[i] != 0) {
		aux[0] = msg[i];
		// Hago la comparación del paso anterior. Esto lo hago para que no rompa en el último paso.
		assert(!checkResponseInner(rData, b, bOut, bTransf));
		writeToBuf(aux, b);
		i++;
	}
	assert(checkResponseInner(rData, b, bOut, bTransf));
	assert(rData->parserState == FINISHED);

	// Al buffer de la transformación espero haber paso solo el contenido de los chunks.
	for (int i = 0; msgTransf[i] != 0; i++) {
		assert(buffer_read(bTransf) == msgTransf[i]);
	}
	assert(buffer_read(bTransf) == 0);
}

static void
insertToBuffer (ResponseData *rData, char *text, buffer *b, buffer *bOut) {
	int i = 0;

	resetData (rData, b, bOut);

	while (text[i] != 0) {
		buffer_write(b, text[i]);
		i++;
	}
}

static void
resetData (ResponseData *rData, buffer *b, buffer *bOut) {
	buffer_reset(b);
	buffer_reset(bOut);
	rData->parserState = VERSION;
	rData->isBufferEmpty = false;
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->status = 0;
	rData->bodyLength = -1;
	rData->isChunked = false;
	rData->withTransf = false;
}
