#include <stdio.h>
#include <unistd.h>
#include <buffer/buffer.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHUNK_SIZE_DIGITS 7 /** This gives us a max. chunk size of 268MB */

ssize_t
write_empty_chunk(buffer * dst_buffer)
{
  size_t dst_buffer_space;
  uint8_t * dst_buffer_ptr = buffer_write_ptr (dst_buffer, &dst_buffer_space);
  size_t chunk_len = strlen("0\r\n\r\n");
  if(dst_buffer_space >= chunk_len) {
    strncpy(dst_buffer_ptr, "0\r\n\r\n", chunk_len);
    buffer_write_adv (dst_buffer, chunk_len);
    return chunk_len;
  } else {
    return 0;
  }
}

ssize_t
dump_chunk_from_fd(int src_fd, buffer * dst_buffer)
{
  ssize_t read_bytes = 0;
  size_t dst_buffer_space;
  uint8_t * dst_buffer_ptr = buffer_write_ptr (dst_buffer, &dst_buffer_space);
  uint8_t * current_dst_buffer_ptr = dst_buffer_ptr;

  if(dst_buffer_space > MAX_CHUNK_SIZE_DIGITS + 4 ) {
    size_t chunk_size = dst_buffer_space - MAX_CHUNK_SIZE_DIGITS - 4;
    uint8_t * temp_buffer = malloc(chunk_size);
    if(temp_buffer == NULL) {
      return -1;
    }
    read_bytes = read(src_fd, temp_buffer, chunk_size);
    if(read_bytes > 0) {
      current_dst_buffer_ptr += sprintf((char *)current_dst_buffer_ptr, "%X\r\n", (unsigned int)read_bytes);
      memcpy(current_dst_buffer_ptr, temp_buffer, read_bytes);
      current_dst_buffer_ptr+=read_bytes;
      current_dst_buffer_ptr += sprintf((char *)current_dst_buffer_ptr, "\r\n");
      buffer_write_adv (dst_buffer, current_dst_buffer_ptr - dst_buffer_ptr);
      printf("Wrote chunk of size:%d\n", current_dst_buffer_ptr - dst_buffer_ptr);
      free(temp_buffer);
      return current_dst_buffer_ptr - dst_buffer_ptr;
    } else {
      return read_bytes;
    }
  }

  return 0;

}