#ifndef MY_PARSER_UTILS_H
#define MY_PARSER_UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <buffer/buffer.h>

/** Leo en mayúscula/minúscula del buffer b y escribo en bOut. */
#define READ_UP_CHAR(b, bOut) toupper(readAndWrite(b, bOut))
#define READ_DOWN_CHAR(b, bOut) tolower(readAndWrite(b, bOut))

/** Miro en mayúscula/minúscula el siguiente elemento a leer de b. */
#define PEEK_UP_CHAR(b) toupper(buffer_peek(b))
#define PEEK_DOWN_CHAR(b) tolower(buffer_peek(b))

// V_2.0 is not supported.
typedef enum {
	UNDEFINED, V_1_0, V_1_1
} httpVersion;

/** Leo de b y escribo en bOut siempre y cuando haya algo para leer
* de b y que no esté en una zona reservada.
*/
uint8_t
readAndWrite (buffer *b, buffer *bOut);

/** Leo todos los siguientes espacios y tabs de b. */
uint8_t
moveThroughSpaces (buffer *b);

/** Escribo toda la data de myBuf en b. */
void
writeToBuf (char *myBuf, buffer *b);

/** Escribo la cantidad especificada de elementos de myBuf (length) en b
* en sentido inverso (ideal para escribir en zona reservada).
*/
void
writeToBufReverse (char *myBuf, buffer *b, int length);

/** Escribo un número en b
* en sentido inverso (ideal para escribir en zona reservada).
*/
void
writeDecToBufReverse (int number, buffer *b);

/** Escribo un número en b pasado a formato hexadecimal y
* en sentido inverso (ideal para escribir en zona reservada).
*/
void
writeHexToBufReverse (int number, buffer *b);

/** Paso la cantidad especificada de elementos de b a bOut.
* Si no tengo suficientes elementos a leer de b en quantity queda
* la cantidad restante y writeToTransfBuf devuelve false.
*/
bool
writeToTransfBuf (buffer *b, buffer *bOut, int *quantity);

/** Como writeToBufReverse pero sin especificar la cantidad de elementos. */
void
writePrefix (buffer *b, char *prefix);

/** Comparo lo siguiente a leer de b con un formato dado y lo paso a bOut a medida que leo.
* Si no terminé la comparación y b queda vacío, pongo bEmpty en true y pongo todo lo que comparé
* hasta el momento en la zona reservada de b. Esto es para que la siguiente vez que convoque matchFormat
* tenga con que comparar y como lo anterior está en la zona reservada esto no se vuelve a pasar a bOut.
* Adicionalmente para cuando el buffer queda vació, puedo escribir un prefijo en la zona reservada.
* Esto resulta útil para cuando hubo operaciones antes del matchFormat que voy a tener que realizar al
* reanudar el parser previo a nueva llamada de matchFormat.
*/
bool
matchFormat (char *format, buffer *b, buffer *bOut, char *prefix, bool *bEmpty);

/** Como matchFormat pero sin escribir en un buffer de salida. */
bool
simpleMatchFormat (char *format, buffer *b, char *prefix, bool *bEmpty);

/** Leo un número de b en formato decimal y lo paso a bOut.
* Si b queda vacío durante la operación, hago algo similar a lo indicado para matchFormat.
*/
bool
getNumber (int *number, buffer *b, buffer *bOut, char *prefix, bool *bEmpty);

/** Leo un número de b en formato hexadecimal y lo paso a bOut.
* Si b queda vacío durante la operación, hago algo similar a lo indicado para matchFormat.
*/
bool
getHexNumber (int *number, buffer *b, buffer *bOut, char *prefix, bool *bEmpty);

/** Como getHexNumber pero sin escribir en un buffer de salida. */
bool
simpleGetHexNumber (int *number, buffer *b, char *prefix, bool *bEmpty);

/** Chequeo si tengo un \n en b y lo paso a bOut.
* Si b queda vacío durante la operación, hago algo similar a lo indicado para matchFormat.
*/
bool
checkLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty);

/** Chequeo si tengo un \r\n en b y lo paso a bOut.
* Si b queda vacío durante la operación, hago algo similar a lo indicado para matchFormat.
*/
bool
checkCRLF (buffer *b, buffer *bOut, char *prefix, bool *bEmpty);

/** Como checkCRLF pero sin escribir en un buffer de salida. */
bool
simpleCheckCRLF (buffer *b, char *prefix, bool *bEmpty);

/** Leo en salida estándar la cantidad especificada de elementos de b. */
bool
writeToStdout (int length, buffer *b);

#endif
