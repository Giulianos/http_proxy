#include "requestParser.c" // Source code included to test static functions.
#include <assert.h>

static void assertMethod (RequestData *rData, buffer *b, buffer *bOut);
static void assertUriHost (RequestData *rData, buffer *b, buffer *bOut);
static void assertUri (RequestData *rData, buffer *b, buffer *bOut);
//static void assertStartLine (RequestData *rData, buffer *b, buffer *bOut);
static void assertRequest (RequestData *rData, buffer *b, buffer *bOut);
static void insertToBuffer (RequestData *rData, char *text, buffer *b, buffer *bOut);
static void resetData (RequestData *rData, buffer *b, buffer *bOut);

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char *argv[]) {
	RequestData *rData = (RequestData *) malloc(sizeof(RequestData));
	struct buffer b;
	uint8_t direct_buff[100];
	struct buffer bOut;
	uint8_t direct_buff_out[100];
	buffer_init(&b, N(direct_buff), direct_buff);
	buffer_init(&bOut, N(direct_buff_out), direct_buff_out);

	if (rData == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	assertMethod(rData, &b, &bOut);
	assertUriHost(rData, &b, &bOut);
	assertUri(rData, &b, &bOut);
//	assertStartLine(rData, &b);
	assertRequest(rData, &b, &bOut);

	free(rData);

	return 0;
}

static void assertMethod (RequestData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "HeA", b, bOut);
	assert(!extractHttpMethod(rData, b, bOut));
	insertToBuffer(rData, "HeAlD", b, bOut);
	assert(!extractHttpMethod(rData, b, bOut));
	assert(rData->method == UNDEFINED_M);

	insertToBuffer(rData, "HeaD", b, bOut);
	assert(extractHttpMethod(rData, b, bOut));
	assert(rData->method == HEAD);

	insertToBuffer(rData, "POST", b, bOut);
	assert(extractHttpMethod(rData, b, bOut));
	assert(rData->method == POST);


	// Método no soportado.
	insertToBuffer(rData, "PUT", b, bOut);
	assert(!extractHttpMethod(rData, b, bOut));
	assert(rData->method == PUT);
}

static void assertUriHost (RequestData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "example.org/", b, bOut);
	checkUriForHost(rData, b, bOut);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "userinfo@example.org/", b, bOut);
	checkUriForHost(rData, b, bOut);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "userinfo@example.org:8080/", b, bOut);
	checkUriForHost(rData, b, bOut);
	assert(strcmp("example.org", rData->host) == 0);
}

static void assertUri (RequestData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "hTtp://example.org/", b, bOut);
	checkUri(rData, b, bOut);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "hTtps://example.org/", b, bOut);
	checkUri(rData, b, bOut);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "hTlp://example.org/", b, bOut);
	assert(!checkUri(rData, b, bOut));

	// Solo devuelvo true si tengo uri absoluto.
	insertToBuffer(rData, "/foo", b, bOut);
	assert(!checkUri(rData, b, bOut));
}

//static void assertStartLine (RequestData *rData, buffer *b, buffer *bOut) {
//	// Método incorrecto
//	insertToBuffer(rData, "gkt /foo HTtP/1.1", b, bOut);
//	assert(!checkStartLine(rData, b, bOut));
//
//	// Versión incorrecto
//	insertToBuffer(rData, "get /foo HTtP/1.4", b, bOut);
//	assert(!checkStartLine(rData, b, bOut));
//
//	insertToBuffer(rData, "gEt /foo HTtP/1.1", b, bOut);
//	assert(checkStartLine(rData, b, bOut));
//	assert(rData->host[0] == 0);
//
//	insertToBuffer(rData, "gEt hTtp://example.org/ HTtP/1.1", b, bOut);
//	assert(checkStartLine(rData, b, bOut));
//	assert(strcmp("example.org", rData->host) == 0);
//}

static void assertRequest (RequestData *rData, buffer *b, buffer *bOut) {
	insertToBuffer(rData, "gEt /foo HTtP/1.1\r\nHost: example.org\r\n", b, bOut);
	assert(checkRequestInner(rData, b, bOut));
	assert(strcmp("example.org", rData->host) == 0);
}

static void insertToBuffer (RequestData *rData, char *text, buffer *b, buffer *bOut) {
	int i = 0;

	resetData (rData, b, bOut);

	while (text[i] != 0) {
		buffer_write(b, text[i]);
		i++;
	}
}

static void resetData (RequestData *rData, buffer *b, buffer *bOut) {
	buffer_reset(b);
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->method = UNDEFINED_M;

	for (int i = 0; i < HOST_MAX_SIZE && rData->host[i] != 0; i++) {
		rData->host[i] = 0;
	}
}
