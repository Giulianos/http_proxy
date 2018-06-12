#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "response_parser.h"

#define BUFFER_SIZE 10

void
response_ended(void * info);

static bool response_complete = false;

int main ()
{
  buffer stdin_buffer;
  uint8_t * stdin_buffer_mem = malloc(BUFFER_SIZE);
  buffer_init (&stdin_buffer, BUFFER_SIZE, stdin_buffer_mem);

  buffer stdout_buffer;
  uint8_t * stdout_buffer_mem = malloc(BUFFER_SIZE);
  buffer_init (&stdout_buffer, BUFFER_SIZE, stdout_buffer_mem);

  struct response_parser_config config = {
      .in_buffer = &stdin_buffer,
      .out_buffer = &stdout_buffer,
      .response_ended_callback = response_ended,
      .callbacks_info = NULL
  };

  response_parser_t parser = response_parser_new (&config);

  ssize_t read_bytes;
  ssize_t written_bytes;
  size_t stdin_buffer_space;
  uint8_t * stdin_buffer_ptr;
  size_t stdout_buffer_size;
  uint8_t * stdout_buffer_ptr;
  int parse_ret_value;

  do {
    stdin_buffer_ptr = buffer_write_ptr (&stdin_buffer, &stdin_buffer_space);
    if(stdin_buffer_space > 0) {
      read_bytes = read(STDIN_FILENO, stdin_buffer_ptr, stdin_buffer_space);
      if(read_bytes > 0) {
        buffer_write_adv (&stdin_buffer, read_bytes);
        parse_ret_value = response_parser_parse (parser);
        if(parse_ret_value < 0) {
          dprintf (STDERR_FILENO, "[PARSER ERROR]\n");
          return 1;
        }
        do {
          stdout_buffer_ptr = buffer_read_ptr (&stdout_buffer, &stdout_buffer_size);
          written_bytes = write (STDOUT_FILENO, stdout_buffer_ptr, stdout_buffer_size);
          if(written_bytes >= 0) {
            buffer_read_adv (&stdout_buffer, written_bytes);
            stdout_buffer_size -= written_bytes;
          } else {
            dprintf (STDERR_FILENO, "[WRITE ERROR]\n");
            return 1;
          }
        } while(stdout_buffer_size > 0);
      } else if(read_bytes == 0) {
        if(!response_complete) {
          dprintf (STDERR_FILENO, "[INVALID REQUEST] EOF but request incomplete\n");
          return 1;
        }
      }
    }
  } while(read_bytes > 0 && parse_ret_value >= 0 && !response_complete);

  dprintf (STDERR_FILENO, "[PARSER DONE]\n");

  return 0;
}

void
response_ended(void * info)
{
  (void)info;

  response_complete = true;
}
