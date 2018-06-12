/**
 * buffer.c - buffer con acceso directo (útil para I/O) que mantiene
 *            mantiene puntero de lectura y de escritura.
 *
 * La implementación del buffer es la de Juan salvo por la función buffer_peek y
 * los cambios para aceptar una zona reservada.
 * Para el resto de las funciones, es todo lo mismo trabajando con infLimit en
 * vez de data.
 */
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "buffer/buffer.h"

inline void
buffer_reset(buffer* b)
{
  b->read = b->infLimit;
  b->write = b->infLimit;
}

void
buffer_init(buffer* b, const size_t n, uint8_t* data)
{
  buffer_init_r(b, 0, n, data);
}

void
buffer_init_r(buffer* b, const size_t n0, const size_t n, uint8_t* data)
{
  b->data = data;
  b->infLimit = b->data + n0;
  buffer_reset(b);
  b->limit = b->data + n;
}

inline bool
buffer_can_write(buffer* b)
{
  return b->limit - b->write > 0;
}

inline uint8_t*
buffer_write_ptr(buffer* b, size_t* nbyte)
{
  assert(b->write <= b->limit);
  *nbyte = b->limit - b->write;
  return b->write;
}

inline bool
buffer_can_read(buffer* b)
{
  return b->write - b->read > 0;
}

inline uint8_t*
buffer_read_ptr(buffer* b, size_t* nbyte)
{
  assert(b->read <= b->write);
  *nbyte = b->write - b->read;
  return b->read;
}

inline void
buffer_write_adv(buffer* b, const ssize_t bytes)
{
  if (bytes > -1) {
    b->write += (size_t)bytes;
    assert(b->write <= b->limit);
  }
}

inline void
buffer_read_adv(buffer* b, const ssize_t bytes)
{
  if (bytes > -1) {
    b->read += (size_t)bytes;
    assert(b->read <= b->write);

    if (b->read == b->write) {
      // compactacion poco costosa
      buffer_compact(b);
    }
  }
}

inline uint8_t
buffer_read(buffer* b)
{
  uint8_t ret;
  if (buffer_can_read(b)) {
    ret = *b->read;
    buffer_read_adv(b, 1);
  } else {
    ret = 0;
  }
  return ret;
}

// Función propia
inline uint8_t
buffer_peek(buffer* b)
{
  return buffer_can_read(b) ? *b->read : 0;
}

// Función propia
inline bool
buffer_can_write_reverse(buffer* b)
{
  return b->read - b->data > 0;
}

// Función propia
inline bool
is_reserved(buffer* b)
{
  return b->read - b->infLimit < 0;
}

// Función propia
inline void
buffer_write_adv_reverse(buffer* b, const ssize_t bytes)
{
  if (bytes > -1) {
    b->read -= (size_t)bytes;
    assert(b->data <= b->read);
  }
}

// Función propia
inline void
buffer_write_reverse(buffer* b, uint8_t c)
{
  if (buffer_can_write_reverse(b)) {
    buffer_write_adv_reverse(b, 1);
    *b->read = c;
  }
}

inline void
buffer_write(buffer* b, uint8_t c)
{
  if (buffer_can_write(b)) {
    *b->write = c;
    buffer_write_adv(b, 1);
  }
}

void
buffer_compact(buffer* b)
{
  if (b->infLimit == b->read) {
    // nada por hacer
  } else if (b->read == b->write) {
    b->read = b->infLimit;
    b->write = b->infLimit;
  } else {
    const size_t n = b->write - b->read;
    memmove(b->infLimit, b->read, n);
    b->read = b->infLimit;
    b->write = b->infLimit + n;
  }
}
