#include "responseParser/responseParser.c" // Source code included to test static functions.
#include <assert.h>

static void assertStartLine (ResponseData *rData, buffer *b);
static void assertContentLength (ResponseData *rData, buffer *b);
static void assertContentEncoding (ResponseData *rData, buffer *b);
static void assertTransferEncoding (ResponseData *rData, buffer *b);
static void assertHeaders (ResponseData *rData, buffer *b);
static void assertBody (ResponseData *rData, buffer *b, buffer *bOut);
static void assertResponse (ResponseData *rData, buffer *b, buffer *bOut);
static void insertToBuffer (ResponseData *rData, char *text, buffer *b);
static void resetData (ResponseData *rData, buffer *b);

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char *argv[]) {
	ResponseData *rData = (ResponseData *) malloc(sizeof(ResponseData));
	struct buffer b;
	uint8_t direct_buff[100];
	struct buffer b2;
	uint8_t direct_buff2[100];
	buffer_init(&b, N(direct_buff), direct_buff);
	buffer_init(&b2, N(direct_buff2), direct_buff2);

	if (rData == NULL) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return 1;
	}

	assertStartLine(rData, &b);
	assertContentLength(rData, &b);
	assertContentEncoding(rData, &b);
	assertTransferEncoding(rData, &b);
	assertHeaders(rData, &b);
	assertBody(rData, &b, &b2);
	assertResponse(rData, &b, &b2);

	free(rData);

	return 0;
}

static void assertStartLine (ResponseData *rData, buffer *b) {
	insertToBuffer(rData, "HTjP/1.1 200 OK", b);
	assert(!checkStartLine(rData, b));
	assert(rData->state == START_LINE_FORMAT_ERROR);

	insertToBuffer(rData, "HTtP/1.3 200 OK", b);
	assert(!checkStartLine(rData, b));
	assert(rData->state == VERSION_ERROR);

	insertToBuffer(rData, "HtTP/1.1 t200 OK", b);
	assert(!checkStartLine(rData, b));
	assert(rData->state == START_LINE_FORMAT_ERROR);
	assert(rData->status == 0);

	insertToBuffer(rData, "HtTP/1.0 200 OK", b);
	assert(checkStartLine(rData, b));
	assert(rData->status == 200);
}

static void assertContentLength (ResponseData *rData, buffer *b) {
	insertToBuffer(rData, "ontent--length: 321", b);
	assert(!checkContentHeader(rData, b));

	insertToBuffer(rData, "untent-length: 321", b);
	assert(!checkContentHeader(rData, b));

	insertToBuffer(rData, "onTent-leNgth:321", b);
	assert(checkContentHeader(rData, b));
	assert(rData->bodyLength == 321);

	insertToBuffer(rData, "onTent-leNgth: \t 55", b);
	assert(checkContentHeader(rData, b));
	assert(rData->bodyLength == 55);

	insertToBuffer(rData, "conTent-leNgth: \t 77", b);
	assert(!checkContentHeader(rData, b));

	insertToBuffer(rData, "conTent-leNgth: \t 77\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->bodyLength == 77);
}

static void assertContentEncoding (ResponseData *rData, buffer *b) {
	insertToBuffer(rData, "onTent--encoDing: chunked", b);
	assert(!checkContentHeader(rData, b));
	assert(rData->isChunked == false);

	insertToBuffer(rData, "onTent-encoDing: chunced", b);
	assert(!checkContentHeader(rData, b));
	assert(rData->isChunked == false);

	insertToBuffer(rData, "onTent-encoDing: \t chunkEd", b);
	assert(checkContentHeader(rData, b));
	assert(rData->isChunked == true);

	insertToBuffer(rData, "onTent-encoDing:chunkEd", b);
	assert(checkContentHeader(rData, b));

	insertToBuffer(rData, "conTent-encoDing:chunkEd\r\n\r\n", b);
	assert(checkHeaders(rData, b));
}

static void assertTransferEncoding (ResponseData *rData, buffer *b) {
	insertToBuffer(rData, "ransfer.encoDing: chunked", b);
	assert(!checkTransferHeader(rData, b));
	assert(rData->isChunked == false);

	insertToBuffer(rData, "ransfer-encoDing: \t chunkEd", b);
	assert(checkTransferHeader(rData, b));
	assert(rData->isChunked == true);

	insertToBuffer(rData, "transfer-encoDing:chunkEd\r\n\r\n", b);
	assert(checkHeaders(rData, b));
}

static void assertHeaders (ResponseData *rData, buffer *b) {
	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\r\ndsd\r\n\n", b);
	assert(!checkHeaders(rData, b));

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\ndsd\r\n\r\n", b);
	assert(checkHeaders(rData, b));

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\ndsd\r\n\r\n", b);
	assert(checkHeaders(rData, b));

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\ndsd\r\ncontent.lenGth:3\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->bodyLength == -1);

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\ndsd\r\ncontent-lenGth:3\r\nconTent-encoding:chunced\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->bodyLength == 3);
	assert(rData->isChunked == false);

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nia\r\ndsd\r\ncontent-lenGth:5\r\nconTent-encoding:chunked\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->bodyLength == 5);
	assert(rData->isChunked == true);

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\ntransfer:chunked\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->isChunked == false);

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\ntransfer-encoding:chunked\r\n\r\n", b);
	assert(checkHeaders(rData, b));
	assert(rData->isChunked == true);
}

static void assertBody (ResponseData *rData, buffer *b, buffer *bOut) {
	// If length is not specified and chunked encoding is disabled I do nothing.
	assert(!extractBody(rData, b, bOut));

	insertToBuffer(rData, "123", b);
	rData->bodyLength = 4;
	assert(!extractBody(rData, b, bOut));

	insertToBuffer(rData, "1234", b);
	rData->bodyLength = 4;
	assert(extractBody(rData, b, bOut));

	insertToBuffer(rData, "4\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n", b);
	rData->isChunked = true;
	assert(extractBody(rData, b, bOut));

	insertToBuffer(rData, "3\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n", b);
	rData->isChunked = true;
	assert(!extractBody(rData, b, bOut));

	insertToBuffer(rData, "5\r\nWiki\r\n5\r\npedia\r\nE\r\n in\r\n\r\nchunks.\r\n0\r\n\r\n", b);
	rData->isChunked = true;
	assert(!extractBody(rData, b, bOut));
};

static void assertResponse (ResponseData *rData, buffer *b, buffer *bOut) {
	responseState state = OK;

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\ntransfer-encoding:chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n", b);
	assert(checkResponse(&state, b, bOut));

	insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTeNt-lengTh:4\r\n\r\nWikipedia", b);
	assert(checkResponse(&state, b, bOut));

	insertToBuffer(rData, "HTkP/1.1 200 OK\r\nconTeNt-lengTh:4\r\n\r\nWikipedia", b);
	assert(!checkResponse(&state, b, bOut));
	assert(state == START_LINE_FORMAT_ERROR);
}

static void insertToBuffer (ResponseData *rData, char *text, buffer *b) {
	int i = 0;

	resetData (rData, b);

	while (text[i] != 0) {
		buffer_write(b, text[i]);
		i++;
	}
}

static void resetData (ResponseData *rData, buffer *b) {
	buffer_reset(b);
	rData->state = OK;
	rData->version = UNDEFINED;
	rData->status = 0;
	rData->bodyLength = -1;
	rData->isChunked = false;
}
