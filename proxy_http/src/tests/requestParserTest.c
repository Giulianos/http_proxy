#include "requestParser.c" // Source code included to test static functions.
#include <assert.h>

static void assertMethod (RequestData *rData, buffer *b);
static void assertUriHost (RequestData *rData, buffer *b);
static void assertUri (RequestData *rData, buffer *b);
static void assertStartLine (RequestData *rData, buffer *b);
static void assertRequest (RequestData *rData, buffer *b);
static void insertToBuffer (RequestData *rData, char *text, buffer *b);
static void resetData (RequestData *rData, buffer *b);

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char *argv[]) {
	RequestData *rData = (RequestData *) malloc(sizeof(RequestData));
	struct buffer b;
	uint8_t direct_buff[100];
	buffer_init(&b, N(direct_buff), direct_buff);

	if (rData == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	assertStartLine(rData, &b);
	assertMethod(rData, &b);
	assertUriHost(rData, &b);
	assertUri(rData, &b);
	assertStartLine(rData, &b);
	assertRequest(rData, &b);

	free(rData);

	return 0;
}

static void assertMethod (RequestData *rData, buffer *b) {
	insertToBuffer(rData, "HeA", b);
	assert(!extractHttpMethod(rData, b));
	insertToBuffer(rData, "HeAlD", b);
	assert(!extractHttpMethod(rData, b));
	assert(rData->method == UNDEFINED_M);

	insertToBuffer(rData, "HeaD", b);
	assert(extractHttpMethod(rData, b));
	assert(rData->method == HEAD);

	insertToBuffer(rData, "PUT", b);
	assert(extractHttpMethod(rData, b));
	assert(rData->method == PUT);
}

static void assertUriHost (RequestData *rData, buffer *b) {
	insertToBuffer(rData, "example.org/", b);
	checkUriForHost(rData, b);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "userinfo@example.org/", b);
	checkUriForHost(rData, b);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "userinfo@example.org:8080/", b);
	checkUriForHost(rData, b);
	assert(strcmp("example.org", rData->host) == 0);
}

static void assertUri (RequestData *rData, buffer *b) {
	insertToBuffer(rData, "hTtp://example.org/", b);
	checkUri(rData, b);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "hTtps://example.org/", b);
	checkUri(rData, b);
	assert(strcmp("example.org", rData->host) == 0);

	insertToBuffer(rData, "hTlp://example.org/", b);
	assert(!checkUri(rData, b));

	// Solo devuelvo true si tengo uri absoluto.
	insertToBuffer(rData, "/foo", b);
	assert(!checkUri(rData, b));
}

static void assertStartLine (RequestData *rData, buffer *b) {
	// Método incorrecto
	insertToBuffer(rData, "gkt /foo HTtP/1.1", b);
	assert(!checkStartLine(rData, b));

	// Versión incorrecto
	insertToBuffer(rData, "get /foo HTtP/1.4", b);
	assert(!checkStartLine(rData, b));

	insertToBuffer(rData, "gEt /foo HTtP/1.1", b);
	assert(checkStartLine(rData, b));
	assert(rData->host[0] == 0);

	insertToBuffer(rData, "gEt hTtp://example.org/ HTtP/1.1", b);
	assert(checkStartLine(rData, b));
	assert(strcmp("example.org", rData->host) == 0);
}

static void assertRequest (RequestData *rData, buffer *b) {
	insertToBuffer(rData, "gEt /foo HTtP/1.1\r\nHost: example.org\r\n", b);
	assert(checkRequestInner(rData, b));
	assert(strcmp("example.org", rData->host) == 0);
}

static void insertToBuffer (RequestData *rData, char *text, buffer *b) {
	int i = 0;

	resetData (rData, b);

	while (text[i] != 0) {
		buffer_write(b, text[i]);
		i++;
	}
}

static void resetData (RequestData *rData, buffer *b) {
	buffer_reset(b);
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->method = UNDEFINED_M;

	for (int i = 0; i < HOST_MAX_SIZE && rData->host[i] != 0; i++) {
		rData->host[i] = 0;
	}
}
