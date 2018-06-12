#include <http_responses/http_responses.h>
#include <buffer/buffer.h>
#include <stdio.h>

int
main() {
  uint8_t buffer_mem[1000];
  buffer temp_buffer;
  buffer_init (&temp_buffer, 1000, buffer_mem);

  http_response_t response = {
      .code = 404,
      .msg = "NOT FOUND",
      .out_buffer = &temp_buffer,
      .written = 0,
      .state = 0,
      .section_index = 0,
  };

  http_responses_write (&response);

  printf("%*s", response.written, buffer_mem);
}