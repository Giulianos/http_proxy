#ifndef BUFFER_H_VelRDAxzvnuFmwEaR0ftrkIinkT
#define BUFFER_H_VelRDAxzvnuFmwEaR0ftrkIinkT

#include <stdbool.h>
#include <unistd.h>  // size_t, ssize_t

/**
 * buffer.c - buffer con acceso directo (útil para I/O) que mantiene
 *            mantiene puntero de lectura y de escritura.
 *
 *
 * Para esto se mantienen dos punteros, uno de lectura
 * y otro de escritura, y se provee funciones para
 * obtener puntero base y capacidad disponibles.
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * |   |   |   |   |   |   |
 * +---+---+---+---+---+---+
 * ↑                       ↑
 * W=0                     limit=6
 *
 * Invariantes:
 *    R <= W <= limit
 *
 * Se quiere escribir en el bufer cuatro bytes.
 *
 * ptr + 0 <- buffer_write_ptr(b, &wbytes), wbytes=6
 * n = read(fd, ptr, wbytes)
 * buffer_write_adv(b, n = 4)
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   |   |
 * +---+---+---+---+---+---+
 *                 ↑       ↑
 *                W=4      limit=6
 *
 * Quiero leer 3 del buffer
 * ptr + 0 <- buffer_read_ptr, wbytes=4
 * buffer_read_adv(b, 3);
 *
 *            R=3
 *             ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   |   |
 * +---+---+---+---+---+---+
 *                 ↑       ↑
 *                W=4      limit=6
 *
 * Quiero escribir 2 bytes mas
 * ptr + 4 <- buffer_write_ptr(b, &wbytes=2);
 * buffer_write_adv(b, 2)
 *
 *            R=3
 *             ↓
 * +---+---+---+---+---+---+
 * | H | O | L | A |   | M |
 * +---+---+---+---+---+---+
 *                         ↑
 *                         limit=6
 *                         W=4
 * Compactación a demanda
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * | A |   | M |   |   |   |
 * +---+---+---+---+---+---+
 *             ↑           ↑
 *            W=3          limit=6
 *
 * Leo los tres bytes, como R == W, se auto compacta.
 *
 * R=0
 * ↓
 * +---+---+---+---+---+---+
 * |   |   |   |   |   |   |
 * +---+---+---+---+---+---+
 * ↑                       ↑
 * W=0                     limit=6
 */
typedef struct buffer buffer;
struct buffer {
    uint8_t *data;

    /** límite inferior del buffer - Agregado a la implementación de Juan.
     * Entre data e infLimit paso a tener una zona reservada.
     * Esta zona la usa cuando voy a querer poner algo al comienzo del buffer
     * moviendo el puntero read dentro de la zona reservada,
     */
    uint8_t *infLimit;

    /** límite superior del buffer. inmutable */
    uint8_t *limit;

    /** puntero de lectura */
    uint8_t *read;

    /** puntero de escritura */
    uint8_t *write;
};

/**
 * inicializa el buffer sin utilizar el heap
 */
void
buffer_init(buffer *b, const size_t n, uint8_t *data);

/**
 * inicializa el buffer sin utilizar el heap.
 * Como la implementación de buffer_init pero con mí límite de zona reservada.
 */
void
buffer_init_r(buffer *b, const size_t n0, const size_t n, uint8_t *data);

/**
 * Retorna un puntero donde se pueden escribir hasta `*nbytes`.
 * Se debe notificar mediante la función `buffer_write_adv'
 */
uint8_t *
buffer_write_ptr(buffer *b, size_t *nbyte);
void
buffer_write_adv(buffer *b, const ssize_t bytes);

uint8_t *
buffer_read_ptr(buffer *b, size_t *nbyte);
void
buffer_read_adv(buffer *b, const ssize_t bytes);

/**
 * obtiene un byte
 */
uint8_t
buffer_read(buffer *b);

/**
 * obtiene un byte sin consumir buffer - función propia
 */
uint8_t
buffer_peek(buffer *b);

/**
 * función propia - como buffer_can_write pero en zona reservada
 * y en sentido inverso.
 */
bool
buffer_can_write_reverse(buffer *b);

/**
 * Veo si el puntero de read está en zona reservada - función propia.
 */
bool
is_reserved(buffer *b);

/**
 * función propia - como buffer_write_adv pero en zona inverso
 * y puede entrar en zona reservada.
 */
void
buffer_write_adv_reverse(buffer *b, const ssize_t bytes);

/**
 * función propia - como buffer_write pero en zona inverso
 * y puede entrar en zona reservada.
 */
void
buffer_write_reverse(buffer *b, uint8_t c);

/** escribe un byte */
void
buffer_write(buffer *b, uint8_t c);

/**
 * compacta el buffer
 */
void
buffer_compact(buffer *b);

/**
 * Reinicia todos los punteros
 */
void
buffer_reset(buffer *b);

/** retorna true si hay bytes para leer del buffer */
bool
buffer_can_read(buffer *b);

/** retorna true si se pueden escribir bytes en el buffer */
bool
buffer_can_write(buffer *b);

#endif
