#include <myParserUtils/myParserUtils.h>

static int hexCharToDec (char c);
static char decToHexChar (int c);

uint8_t
readAndWrite (buffer *b, buffer *bOut) {
	// Si estoy leyendo algo de la zona reservada ya lo escribe al buffer
	// de salida previamente.
	bool isReserved = is_reserved(b);
	uint8_t c = buffer_read(b);

	if (!isReserved && c != 0) {
		buffer_write(bOut, c);
	}

	return c;
}

uint8_t
moveThroughSpaces (buffer *b) {
	uint8_t c;

	while ((c = buffer_peek(b)) == ' ' || c == '\t') {
		buffer_read(b);
	};
	return c;
}

void
writeToBuf (char *myBuf, buffer *b) {
	int i = 0;

	while (myBuf[i] != 0) {
		buffer_write(b, myBuf[i]);
		i++;
	}
}

void
writeToBufReverse (char *myBuf, buffer *b, int length) {
	while (length > 0) {
		length--;
		buffer_write_reverse(b, myBuf[length]);
	}
}

void
writeDecToBufReverse (int number, buffer *b) {
	if (number == 0) {
		buffer_write_reverse(b, '0');
	}

	while (number > 0) {
		buffer_write_reverse(b, number%10 + '0');
		number = number/10;
	}
}

void
writeHexToBufReverse (int number, buffer *b) {
	if (number == 0) {
		buffer_write_reverse(b, '0');
	}

	while (number > 0) {
		buffer_write_reverse(b, decToHexChar(number));
		number = number/16;
	}
}

bool
writeToTransfBuf (buffer *b, buffer *bOut, int *quantity) {
	int auxQuantity = *quantity;

	while (auxQuantity > 0) {
		if (readAndWrite(b, bOut) == 0) {
			break;
		}
		auxQuantity--;
	}
	*quantity = auxQuantity;

	return auxQuantity <= 0;
}

void
writePrefix (buffer *b, char *prefix) {
	writeToBufReverse (prefix, b, strlen(prefix));
}

bool
matchFormat (char *format, buffer *b, buffer *bOut, char *prefix, bool *bEmpty) {
	uint8_t c;
	int i = 0;

	while (format[i] != 0) {
		if ((c = PEEK_UP_CHAR(b)) != toupper(format[i])) { // Incluye escenario en que c es 0.
			break;
		}
		readAndWrite(b, bOut);
		i++;
	}

	if (format[i] != 0 && c == 0) {
		*bEmpty = true;

		writeToBufReverse (format, b, i);
		writePrefix (b, prefix);

		return false;
	}

	return format[i] == 0;
}

bool
simpleMatchFormat (char *format, buffer *b, char *prefix, bool *bEmpty) {
	uint8_t c;
	int i = 0;

	while (format[i] != 0) {
		if ((c = PEEK_UP_CHAR(b)) != toupper(format[i])) { // Incluye escenario en que c es 0.
			break;
		}
		buffer_read(b);
		i++;
	}

	if (format[i] != 0 && c == 0) {
		*bEmpty = true;

		writeToBufReverse (format, b, i);
		writePrefix (b, prefix);

		return false;
	}

	return format[i] == 0;
}

bool
getNumber (int *number, buffer *b, buffer *bOut, char *prefix, bool *bEmpty) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (c != 0 && !isdigit(c)) {
		return false;
	}

	while (isdigit((c = buffer_peek(b)))) {
		currentNum = (10 * currentNum) + (c - '0');
		readAndWrite(b, bOut);
	}

	// Si paso a no tener nada en el buffer no si me faltan leer números.
	if (c == 0) {
		*bEmpty = true;

		writeDecToBufReverse(currentNum, b);
		writePrefix (b, prefix);

		return false;
	}

	*number = currentNum;

	return true;
}

bool
getHexNumber (int *number, buffer *b, buffer *bOut, char *prefix, bool *bEmpty) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (c != 0 && !isxdigit(c)) {
		return false;
	}

	while (isxdigit((c = buffer_peek(b)))) {
		currentNum = (16 * currentNum) + hexCharToDec(c);
		readAndWrite(b, bOut);
	}

	// Si paso a no tener nada en el buffer no si me faltan leer números.
	if (c == 0) {
		*bEmpty = true;

		writeHexToBufReverse(currentNum, b);
		writePrefix (b, prefix);

		return false;
	}

	*number = currentNum;

	return true;
}

bool
simpleGetHexNumber (int *number, buffer *b, char *prefix, bool *bEmpty) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (c != 0 && !isxdigit(c)) {
		return false;
	}

	while (isxdigit((c = buffer_peek(b)))) {
		currentNum = (16 * currentNum) + hexCharToDec(c);
		buffer_read(b);
	}

	// Si paso a no tener nada en el buffer no si me faltan leer números.
	if (c == 0) {
		*bEmpty = true;

		writeHexToBufReverse(currentNum, b);
		writePrefix (b, prefix);

		return false;
	}

	*number = currentNum;

	return true;
}

static int
hexCharToDec (char c) {
	if (isdigit(c)) {
		return c - '0';
	}
	return toupper(c) - 'A' + 10;
}

static char
decToHexChar (int c) {
	if (c < 10) {
		return c + '0';
	}
	return c + 'A' - 10;
}

bool
checkLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty) {
	return matchFormat("\n", b, bOut, prefix, bEmpty);
}

bool
checkCRLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty) {
	return matchFormat("\r\n", b, bOut, prefix, bEmpty);
}

bool
simpleCheckCRLF (buffer *b, char *prefix, bool *bEmpty) {
	return simpleMatchFormat("\r\n", b, prefix, bEmpty);
}

bool
writeToStdout (int length, buffer *b) {
	uint8_t c;

	for (int i = 0; i < length; i++) {
		if ((c = buffer_read(b)) == 0) {
			return false;
		}
		putchar(c);
	}
	return true;
}
