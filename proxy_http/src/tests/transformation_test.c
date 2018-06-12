#include <stdio.h>
#include <string.h>
#include <transformations/transformations.h>
#include <unistd.h>

int
main(int argc, char* argv[])
{
  int transf_in;
  int transf_out;

  char buffer[] = "Hola mundo!";
  char buffer_out[20];
  size_t buffer_size = sizeof(buffer);

  size_t wrote = 0;
  ssize_t written;

  size_t compared = 0;
  ssize_t bytes_read;

  if (transformations_new(&transf_in, &transf_out) < 0) {
    perror("Creating transrformation");
    return 1;
  }

  while (wrote < buffer_size) {
    written = write(transf_in, buffer + wrote, buffer_size - wrote);
    if (written < 0) {
      perror("Writing to transf.");
      return 1;
    }
    wrote += written;
  }

  close(transf_in);

  while (compared < buffer_size) {
    bytes_read = read(transf_out, buffer_out, sizeof(buffer_out));
    if (bytes_read < 0) {
      perror("Reading from trasnf.");
      return 1;
    }
    if (strncmp(buffer_out, buffer + compared, bytes_read) != 0) {
      perror("Test failed, buffers differ.");
      return 1;
    }
    compared += bytes_read;
    write(STDIN_FILENO, buffer_out, bytes_read);
  }

  close(transf_out);

  printf("transformations test ok!\n");

  return 0;
}
