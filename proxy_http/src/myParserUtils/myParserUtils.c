#include "myParserUtils.h"

static int hexCharToDec (char c);

void moveThroughSpaces (buffer *b) {
	uint8_t c;

	while ((c = buffer_peek(b)) == ' ' || c == '\t') {
		buffer_read(b);
	};
}

bool writeToBuf (char *buf, int bufSize, buffer *b) {
	uint8_t c = 0;

	if (bufSize <= 0) { // Sí bufSize <= 0 no escribo nada por lo que nada salió mal.
		return true;
	}

	for (int i = 0; i < bufSize; i++) {
		if ((c = buffer_read(b)) == 0) {
			break;
		}
		buf[i] = c;
	}

	return c != 0;
}

bool writeToTransfBuf (buffer *b, buffer *bOut, int quantity) {
	uint8_t c = 0;

	if (quantity <= 0) { // Sí quantity <= 0 no escribo nada por lo que nada salió mal.
		return true;
	}

	for (int i = 0; i < quantity; i++) {
		if ((c = buffer_read(b)) == 0) {
			break;
		}
		buffer_write(bOut, c);
	}

	return c != 0;

}

bool matchFormat (char *format, buffer *b) {
	int i = 0;

	while (format[i] != 0) {
		if (PEEK_UP_CHAR(b) != toupper(format[i])) { // Incluye escenario en que c es 0.
			break;
		}
		buffer_read(b);
		i++;
	}

	return format[i] == 0;
}

bool getNumber (int *number, buffer *b) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (!isdigit(c)) { // Incluye escenario en que c es 0.
		return false;
	}

	do {
		currentNum = (10 * currentNum) + (c - '0');
		buffer_read(b);
	} while (isdigit((c = buffer_peek(b))));

	*number = currentNum;

	return true;
}

bool getHexNumber (int *number, buffer *b) {
	uint8_t c = buffer_peek(b);
	int currentNum = 0;

	if (!isxdigit(c)) { // Incluye escenario en que c es 0.
		return false;
	}

	do {
		currentNum = (16 * currentNum) + hexCharToDec(c);
		buffer_read(b);
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

bool checkEmptyLine (buffer *b) {
	uint8_t c;

	// Supongo que acepto \r\n o \n para final de línea.
	if ((c = buffer_peek(b)) == '\r') {
		buffer_read(b);
		c = buffer_peek(b);
	}

	if (c != '\n') {
		return false;
	}
	buffer_read(b);

	return true;
}

bool checkLF (buffer *b) {
	uint8_t c;

	if ((c = buffer_peek(b)) != '\n') {
		return false;
	}
	buffer_read(b);

	return true;
}


bool checkCRLF (buffer *b) {
	uint8_t c;

	if ((c = buffer_peek(b)) != '\r') {
		return false;
	}
	buffer_read(b);

	if ((c = buffer_peek(b)) != '\n') {
		return false;
	}
	buffer_read(b);

	return true;
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
