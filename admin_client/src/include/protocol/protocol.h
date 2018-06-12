#ifndef PROTOCOL_H
#define PROTOCOL_H

#define ADMIN_PORT 9090
#define MAX_READ 1024
enum type
{
  SEND_CRED = 0,
  LIST_METRICS,
  LIST_CONFIGS,
  GET_METRIC,
  GET_CONFIG,
  SET_CONFIG,
  ERROR
};

enum error_type
{
  CONFIG_NOT_FOUND = 0,
  METRIC_NOT_FOUND,
  CONFIG_NOT_SET,
  INVALID_LENGTH,
  UNEXPECTED_ERROR
};

/** error types */
#define NO_METRIC 0
#define NO_CONFIG 1

#define MAX_MSG_SIZE 1024

typedef struct
{
  unsigned char type;
  unsigned char param;
  unsigned int buffer_size;
  unsigned char* buffer;
} msg_t;

#endif
