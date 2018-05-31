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
uint8_t moveThroughSpaces (buffer *b);
void writeToBuf (char *myBuf, buffer *b);
void writeToBufReverse (char *myBuf, buffer *b, int length);
bool writeToTransfBuf (buffer *b, buffer *bOut, int quantity);
bool matchFormat (char *format, buffer *b, buffer *bOut, char *prefix, bool *bEmpty);
bool getNumber (int *number, buffer *b, buffer *bOut, char *prefix, bool *bEmpty);
bool getHexNumber (int *number, buffer *b, buffer *bOut);
bool checkLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty);
bool checkCRLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty);
bool writeToStdout (int length, buffer *b);

#endif
