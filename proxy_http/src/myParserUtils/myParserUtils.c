#include <myParserUtils/myParserUtils.h>

static int hexCharToDec (char c);
static void writePrefix (buffer *b, char *prefix);

uint8_t readAndWrite (buffer *b, buffer *bOut) {
	// Si estoy leyendo algo de la zona reservada ya lo escribe al buffer
	// de salida previamente.
	bool isReserved = is_reverse(b);
	uint8_t c = buffer_read(b);

	if (!isReserved && c != 0) {
		buffer_write(bOut, c);
	}

	return c;
}

void moveThroughSpaces (buffer *b, buffer *bOut) {
	uint8_t c;

	while ((c = buffer_peek(b)) == ' ' || c == '\t') {
		readAndWrite(b, bOut);
	};
}

void writeToBuf (char *myBuf, buffer *b) {
	int i = 0;

	while (myBuf[i] != 0) {
		buffer_write(b, myBuf[i]);
		i++;
	}
}

void writeToBufReverse (char *myBuf, buffer *b, int length) {
	while (length > 0) {
		length--;
		buffer_write_reverse(b, myBuf[length]);
	}
}

bool writeToTransfBuf (buffer *b, buffer *bOut, int quantity) {
	uint8_t c = 0;

	if (quantity <= 0) { // Sí quantity <= 0 no escribo nada por lo que nada salió mal.
		return true;
	}

	for (int i = 0; i < quantity; i++) {
		if ((c = readAndWrite(b, bOut)) == 0) {
			break;
		}
	}

	return c != 0;

}

bool matchFormat (char *format, buffer *b, buffer *bOut, char *prefix) {
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
		while (i > 0) { // Vuelvo a escribir en el buffer lo que consumí en la función.
			i--;
			buffer_write_reverse(b, format[i]);
		}
		writePrefix (b, prefix);

		return false;
	}

	return format[i] == 0;
}

bool getNumber (int *number, buffer *b, buffer *bOut, char *prefix) {
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
		while (currentNum > 0) {
			buffer_write_reverse(b, currentNum%10 + '0');
			currentNum = currentNum/10;
		}
		writePrefix (b, prefix);

		return false;
	}

	*number = currentNum;

	return true;
}

static void writePrefix (buffer *b, char *prefix) {
	int prefixLength = strlen(prefix);

	while (prefixLength > 0) {
		prefixLength--;
		buffer_write_reverse(b, prefix[prefixLength]);
	}
}

bool getHexNumber (int *number, buffer *b, buffer *bOut) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (!isxdigit(c)) { // Incluye escenario en que c es 0.
		return false;
	}

	do {
		currentNum = (16 * currentNum) + hexCharToDec(c);
		readAndWrite(b, bOut);
	} while (isxdigit((c = buffer_peek(b))));

	*number = currentNum;

	return true;
}

static int hexCharToDec (char c) {
	if (isdigit(c)) {
		return c - '0';
	}
	return toupper(c) - 'A' + 10;
}

bool checkLF (buffer *b, buffer *bOut, char *prefix) {
	return matchFormat ("\n", b, bOut, prefix);
}


bool checkCRLF (buffer *b, buffer *bOut, char *prefix) {
	return matchFormat ("\r\n", b, bOut, prefix);
}

bool writeToStdout (int length, buffer *b) {
	uint8_t c;

	for (int i = 0; i < length; i++) {
		if ((c = buffer_read(b)) == 0) {
			return false;
		}
		putchar(c);
	}

	return true;
}
