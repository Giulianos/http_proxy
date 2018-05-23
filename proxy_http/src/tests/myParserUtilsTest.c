#include "myParserUtils.h"
#include <assert.h>
#include <stdlib.h>

static void assertSpaces (buffer *b);
static void assertWriteToBuf (buffer *b);
static void assertFormat (buffer *b);
static void assertNumber (buffer *b);
static void assertHexNumber (buffer *b);
static void assertEmptyLine (buffer *b);

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char *argv[]) {
	struct buffer b;
	uint8_t direct_buff[30];
	buffer_init(&b, N(direct_buff), direct_buff);

	assertSpaces(&b);

	buffer_reset(&b);
	assertWriteToBuf(&b);

	buffer_reset(&b);
	assertFormat(&b);

	buffer_reset(&b);
	assertNumber(&b);

	buffer_reset(&b);
	assertHexNumber(&b);

	buffer_reset(&b);
	assertEmptyLine(&b);

	return 0;
}

static void assertSpaces (buffer *b) {
	buffer_write(b, ' ');
	buffer_write(b, ' ');
	buffer_write(b, 'a');
	moveThroughSpaces(b);
	assert(buffer_read(b) == 'a');
	buffer_write(b, ' ');
	buffer_write(b, '2');
	moveThroughSpaces(b);
	assert(buffer_read(b) == '2');
	buffer_write(b, ' ');
	buffer_write(b, '\t');
	buffer_write(b, ' ');
	buffer_write(b, 'b');
	moveThroughSpaces(b);
	assert(buffer_read(b) == 'b');
}

static void assertWriteToBuf (buffer *b) {
	char buf[2] = {0};

	assert(buf[0] == 0);
	assert(buf[1] == 0);
	buffer_write(b, 'b');
	buffer_write(b, 'a');
	assert(writeToBuf(buf, sizeof(buf)/sizeof(buf[0]), b));
	assert(buf[0] == 'b');
	assert(buf[1] == 'a');
	buffer_write(b, 'c');
	assert(writeToBuf(buf, 1, b));
	assert(buf[0] == 'c');
	assert(buf[1] == 'a');
}

static void assertFormat (buffer *b) {
	buffer_write(b, 't');
	buffer_write(b, 'E');
	buffer_write(b, 's');
	buffer_write(b, 'T');
	assert(matchFormat("T", b));
	assert(!matchFormat("Ea", b));
	assert(matchFormat("st", b));
}

static void assertNumber (buffer *b) {
	int number;

	buffer_write(b, 'g');
	assert(!getNumber(&number, b));
	assert(buffer_read(b) == 'g');
	buffer_write(b, '5');
	buffer_write(b, '4');
	buffer_write(b, 'a');
	assert(getNumber(&number, b));
	assert(number == 54);
	assert(buffer_read(b) == 'a');
}

static void assertHexNumber (buffer *b) {
	int number;

	buffer_write(b, 'g');
	assert(!getHexNumber(&number, b));
	assert(buffer_read(b) == 'g');
	buffer_write(b, '8');
	assert(getHexNumber(&number, b));
	assert(number == 8);
	buffer_write(b, 'c');
	assert(getHexNumber(&number, b));
	assert(number == 12);
	buffer_write(b, 'C');
	assert(getHexNumber(&number, b));
	assert(number == 12);
	buffer_write(b, '1');
	buffer_write(b, 'b');
	assert(getHexNumber(&number, b));
	assert(number == 27);
}

static void assertEmptyLine (buffer *b) {
	buffer_write(b, 'a');
	assert(!checkEmptyLine(b));
	assert(!checkEmptyLine(b)); // To check the 'a' stays in buffer.
	buffer_read(b);
	buffer_write(b, '\n');
	assert(checkEmptyLine(b));
	buffer_write(b, '\n');
	buffer_write(b, '\r');
	assert(checkEmptyLine(b));
	buffer_write(b, '\n');
	assert(!checkLF(b));
	assert(checkCRLF(b));
}
