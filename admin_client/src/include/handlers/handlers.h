#ifndef HANDLERS_H
#define HANDLERS_H

#define STDOUT 0
#define STDIN 1
static int admin_socket;

void
admin_read_handler();
void
admin_write_handler();
void
stdin_read_handler();
void
stdout_write_handler();

#endif
