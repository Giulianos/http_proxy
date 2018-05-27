#ifndef MY_PARSER_UTILS_H
#define MY_PARSER_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <buffer/buffer.h>

#define READ_UP_CHAR(b, bOut) toupper(readAndWrite(b, bOut))
#define READ_DOWN_CHAR(b, bOut) tolower(readAndWrite(b, bOut))
#define PEEK_UP_CHAR(b) toupper(buffer_peek(b))
#define PEEK_DOWN_CHAR(b) tolower(buffer_peek(b))

uint8_t readAndWrite (buffer *b, buffer *bOut);
void moveThroughSpaces (buffer *b, buffer *bOut);
void writeToBuf (char *myBuf, buffer *b);
void writeToBufReverse (char *myBuf, buffer *b, int length);
bool writeToTransfBuf (buffer *b, buffer *bOut, int quantity);
bool matchFormat (char *format, buffer *b, buffer *bOut, char *prefix);
bool getNumber (int *number, buffer *b, buffer *bOut, char *prefix);
bool getHexNumber (int *number, buffer *b, buffer *bOut);
bool checkLF (buffer *b, buffer *bOut, char *prefix);
bool checkCRLF (buffer *b, buffer *bOut, char *prefix);
bool writeToStdout (int length, buffer *b);

#endif
