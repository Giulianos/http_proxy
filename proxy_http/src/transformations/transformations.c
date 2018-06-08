#include <transformations/transformations.h>
#include <unistd.h>
#include <stdlib.h>

/** TODO (Config): get name of the program from config module */

/**
 *
 *         (fd_in)
 *         pipe_in[1]   pipe_in[0]
 *   +-----+  +------------>  +-------+
 *   |PROXY|                  |TRANSF.|
 *   +-----+  <------------+  +-------+
 *         pipe_out[0]  pipe_out[1]
 *        (fd_out)
 *
 */

int
transformations_new(int * fd_in, int * fd_out)
{
  int pipe_in[2];
  int pipe_out[2];
  int rc;

  rc = pipe(pipe_in);
  if(rc < 0)
    return -1;

  rc = pipe(pipe_out);
  if(rc < 0) {
    close(pipe_in[0]);
    close(pipe_in[1]);
    return -1;
  }

  switch(fork()) {
    case 0: /** TRANSF. */
      close(pipe_in[1]);
      close(pipe_out[0]);
      pipe_out[0] = pipe_in[1] = -1;

      dup2(pipe_in[0], STDIN_FILENO);
      dup2(pipe_out[1], STDOUT_FILENO);

      /** TODO: transf should be gotten from config module */
      if(-1 == execl("/bin/sh", "sh", "-c", "cat", (char *) 0)) {
        close(pipe_in[0]);
        close(pipe_out[1]);
        return -1;
      }

      exit(1);

    case -1: /** Error */
      close(pipe_out[0]);
      close(pipe_out[1]);
      close(pipe_in[0]);
      close(pipe_in[1]);
      return -1;

    default: /** PROXY */
      close(pipe_in[0]);
      close(pipe_out[1]);

      *fd_in = pipe_in[1];
      *fd_out = pipe_out[0];
      return 0;
  }
}