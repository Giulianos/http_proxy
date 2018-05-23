#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "buffer.h"

#define READ_UP_CHAR(b) toupper(buffer_read(b))
#define READ_DOWN_CHAR(b) tolower(buffer_read(b))
#define PEEK_UP_CHAR(b) toupper(buffer_peek(b))

void moveThroughSpaces (buffer *b);
bool writeToBuf (char *buf, int bufSize, buffer *b);
bool writeToTransfBuf (buffer *b, buffer *bOut, int quantity);
bool matchFormat (char *format, buffer *b);
bool getNumber (int *number, buffer *b);
bool getHexNumber (int *number, buffer *b);
bool checkEmptyLine (buffer *b);
bool checkLF (buffer *b);
bool checkCRLF (buffer *b);
bool writeToStdout (int length, buffer *b);

#endif
