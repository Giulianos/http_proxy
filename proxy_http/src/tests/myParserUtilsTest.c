#include <myParserUtils/myParserUtils.h>
#include <assert.h>
#include <stdlib.h>

static void
assertReadAndWrite (buffer *b, buffer *bOut);
static void
assertReadAndWriteWithZero (buffer *b, buffer *bOut);
static void
assertWriteToBuffer (buffer *bOut);
static void
assertSpaces (buffer *b);
static void
assertWriteToBufferReverse (buffer *b);
static void
assertWriteToTransfBuf (buffer *b, buffer *bOut);
static void
assertWriteToTransfBufWithZero (buffer *b, buffer *bOut);
static void
assertFormatNormal (buffer *b, buffer *bOut);
static void
assertFormatMissing (buffer *b, buffer *bOut);
static void
assertFormatPrefix (buffer *b, buffer *bOut);
static void
assertNumber (buffer *b, buffer *bOut);
static void
assertHexNumber (buffer *b, buffer *bOut);
static void
assertWriteHexToBufReverse (buffer *b);
static void
reset (buffer *b, buffer *bOut);

int
main (int argc, char *argv[]) {
	struct buffer b;
	uint8_t direct_buff[100];
	int totalSpace = 100;
	int reservedSpace = 10;
	struct buffer bOut;
	uint8_t direct_buff_out[30];
	int totalSpaceOut = 30;
	buffer_init_r(&b, reservedSpace, totalSpace, direct_buff);
	buffer_init(&bOut, totalSpaceOut, direct_buff_out);

	assertReadAndWrite(&b, &bOut);

	reset(&b, &bOut);
	assertReadAndWriteWithZero(&b, &bOut);

	reset(&b, &bOut);
	assertWriteToBuffer(&bOut);

	reset(&b, &bOut);
	assertSpaces(&b);

	reset(&b, &bOut);
	assertWriteToBufferReverse (&b);

	reset(&b, &bOut);
	assertWriteToTransfBuf(&b, &bOut);

	reset(&b, &bOut);
	assertWriteToTransfBufWithZero(&b, &bOut);

	reset(&b, &bOut);
	assertFormatNormal(&b, &bOut);

	reset(&b, &bOut);
	assertFormatMissing(&b, &bOut);

	reset(&b, &bOut);
	assertFormatPrefix(&b, &bOut);

	reset(&b, &bOut);
	assertNumber(&b, &bOut);

	reset(&b, &bOut);
	assertHexNumber(&b, &bOut);

	reset(&b, &bOut);
	assertWriteHexToBufReverse(&b);

	return 0;
}

static void
assertReadAndWrite (buffer *b, buffer *bOut) {
	assert(readAndWrite(b, bOut) == 0);

	buffer_write_reverse(b, 'H');
	buffer_write(b, 'O');
	buffer_write(b, 'L');

	assert(readAndWrite(b, bOut) == 'H');
	// Si leo algo de la zona reservada de b, no lo paso a bOut.
	assert(buffer_read(bOut) == 0);
	// Si leo algo de la zona normal de b, lo paso a bOut.
	assert(readAndWrite(b, bOut) == 'O');
	assert(buffer_read(bOut) == 'O');
	assert(readAndWrite(b, bOut) == 'L');
	assert(buffer_read(bOut) == 'L');
}

static void
assertReadAndWriteWithZero (buffer *b, buffer *bOut) {
	bool bEmpty = false;

	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 0);
	buffer_write_reverse(b, 0);
	buffer_write_reverse(b, 'H');
	buffer_write(b, 'O');
	buffer_write(b, 0);
	buffer_write(b, 'L');

	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 'H'); // Elementos zona reservada
	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 0);
	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 'O'); // Elementos zona normal
	assert(buffer_read(bOut) == 'O');
	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 0);
	assert(buffer_read(bOut) == 0);
	assert(readAndWriteWithZero(b, bOut, &bEmpty) == 'L');
	assert(buffer_read(bOut) == 'L');
}

static void
assertWriteToBuffer (buffer *bOut) {
	char *msg = "Hola";
	writeToBuf(msg, bOut);

	for (int i = 0; msg[i] != 0; i++) {
		assert(buffer_read(bOut) == msg[i]);
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertSpaces (buffer *b) {
	writeToBuf (" a", b);
	moveThroughSpaces(b);
	assert(buffer_read(b) == 'a');

	writeToBuf ("\t 2", b);
	moveThroughSpaces(b);
	assert(buffer_read(b) == '2');

	writeToBuf (" \t b", b);
	moveThroughSpaces(b);
	assert(buffer_read(b) == 'b');
}

static void
assertWriteToBufferReverse (buffer *b) {
	char *msg = "Hola";
	writeToBufReverse(msg, b, 4);

	for (int i = 0; msg[i] != 0; i++) {
		assert(buffer_read(b) == msg[i]);
	}
	assert(buffer_read(b) == 0);
}

static void
assertWriteToTransfBuf (buffer *b, buffer *bOut) {
	char *msg = "Hola";
	int length = strlen(msg);

	writeToBuf(msg, b);
	assert(writeToTransfBuf(b, bOut, &length));

	for (int i = 0; msg[i] != 0; i++) {
		assert(buffer_read(bOut) == msg[i]);
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertWriteToTransfBufWithZero (buffer *b, buffer *bOut) {
	char *msg = "H\0la";
	bool bEmpty = false;
	int i, length = 4;

	buffer_write(b, 'H');
	buffer_write(b, '\0');
	buffer_write(b, 'l');
	buffer_write(b, 'a');

	assert(writeToTransfBufWithZero(b, bOut, &length, &bEmpty));

	i = 0;
	while (buffer_can_read(bOut)) {
		assert(buffer_read(bOut) == msg[i]);
		i++;
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertFormatNormal (buffer *b, buffer *bOut) {
	bool bEmpty = false;
	char *msg = "Hola";
	char *msg2 = "HOlA";
	writeToBuf(msg2, b);
	assert(matchFormat(msg, b, bOut, "", &bEmpty));

	for (int i = 0; msg[i] != 0; i++) {
		assert(toupper(buffer_read(bOut)) == toupper(msg[i]));
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertFormatMissing (buffer *b, buffer *bOut) {
	bool bEmpty = false;
	char *msg = "Hola";
	char *msg2 = "HOl";
	writeToBuf(msg2, b);
	assert(!matchFormat(msg, b, bOut, "", &bEmpty));
	assert(bEmpty == true);
	buffer_write(b, 'a');
	assert(matchFormat(msg, b, bOut, "", &bEmpty));

	for (int i = 0; msg[i] != 0; i++) {
		assert(toupper(buffer_read(bOut)) == toupper(msg[i]));
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertFormatPrefix (buffer *b, buffer *bOut) {
	bool bEmpty = false;
	char *msg = "Mundo";
	char *msg2 = "MUnd";
	char *msg3 = "Hola Mundo";
	char *prefix = "HolA ";
	writeToBuf(msg2, b);
	// Paso a tener "MUnd" en zona normal de b.
	assert(!matchFormat(msg, b, bOut, prefix, &bEmpty));
	assert(bEmpty == true);
	// Paso a tener "MUnd" en zona reservada de b y "MUnd" en zona normal de bOut.
	buffer_write(b, 'O');
	// Paso a tener "MUnd" en zona reservada de b y "O" en zona normal.
	assert(matchFormat(msg3, b, bOut, "", &bEmpty));
	// Paso a tener "MUndO" en zona normal de bOut y nada en b.

	for (int i = 0; msg[i] != 0; i++) {
		assert(toupper(buffer_read(bOut)) == toupper(msg[i]));
	}
	assert(buffer_read(bOut) == 0);
}

static void
assertNumber (buffer *b, buffer *bOut) {
	bool bEmpty = false;
	int number;

	buffer_write(b, 'g');
	assert(!getNumber(&number, b, bOut, "", &bEmpty));
	assert(buffer_read(b) == 'g');
	writeToBuf ("54a", b);
	assert(getNumber(&number, b, bOut, "", &bEmpty));
	assert(number == 54);
	assert(buffer_read(b) == 'a');

	writeToBuf ("51", b);
	// Retorno false porque no sé si terminé de leer número.
	assert(!getNumber(&number, b, bOut, "1", &bEmpty));
	assert(bEmpty == true);
	buffer_write(b, 'a');
	assert(getNumber(&number, b, bOut, "", &bEmpty));
	// Leo 51 con el 1 por el prefix anterior.
	assert(number == 151);
}

static void
assertHexNumber (buffer *b, buffer *bOut) {
	bool bEmpty = false;
	int number;

	buffer_write(b, 'g');
	assert(!getHexNumber(&number, b, bOut, "", &bEmpty));
	assert(buffer_read(b) == 'g');
	writeToBuf ("1A ", b); // Pongo un espacio para saber que terminó el número.
	assert(getHexNumber(&number, b, bOut, "", &bEmpty));
	assert(number == 26);
	buffer_read(b);

	writeToBuf ("A", b);
	// Retorno false porque no sé si terminé de leer número.
	assert(!getHexNumber(&number, b, bOut, "1", &bEmpty));
	assert(bEmpty == true);
	buffer_write(b, ' ');
	assert(getHexNumber(&number, b, bOut, "", &bEmpty));
	// Leo 26 (1A en hexa) con el 1 por el prefix anterior.
	assert(number == 26);
	buffer_read(b);
	buffer_read(b);

	writeToBuf ("F ", b);
	assert(getHexNumber(&number, b, bOut, "", &bEmpty));
	assert(number == 15);
}

static void
assertWriteHexToBufReverse (buffer *b) {
	int number = 5;

	assert(buffer_read(b) == 0); // Verifico que el buffer inicialmente esté vacío.
	writeHexToBufReverse (number, b);
	assert(buffer_read(b) == '5');
	assert(buffer_read(b) == 0);

	number = 15;
	writeHexToBufReverse (number, b);
	assert(buffer_read(b) == 'F');
	assert(buffer_read(b) == 0);

	number = 31;
	writeHexToBufReverse (number, b);
	assert(buffer_read(b) == '1');
	assert(buffer_read(b) == 'F');
	assert(buffer_read(b) == 0);
}

static void
reset (buffer *b, buffer *bOut) {
	buffer_reset(b);
	buffer_reset(bOut);
}
