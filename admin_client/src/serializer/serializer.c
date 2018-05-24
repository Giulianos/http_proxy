#include <serializer/serializer.h>
#include <protocol/protocol.h>
#include <stdlib.h>

/** serializers */

unsigned char *
serialize_int(unsigned char * buffer, int value)
{
  buffer[0] = value >> 24;
  buffer[1] = value >> 16;
  buffer[2] = value >> 8;
  buffer[3] = value;
  return buffer + 4;
}

unsigned char *
serialize_char(unsigned char * buffer, char value)
{
  buffer[0] = value;
  return buffer + 1;
}

unsigned char *
serialize_string(unsigned char * buffer, char * str)
{
  do {
    buffer = serialize_char(buffer, *str);
  } while(*str++ != '\0');
  return buffer;
}
/**
 * Serializes the corresponding fields for the type of the msg
 */
unsigned char *
serialize_msg(unsigned char * buffer, msg_t * msg)
{
  buffer = serialize_int(buffer, msg->bytes);
  buffer = serialize_char(buffer, msg->type);
  if(msg->type == GET_METRIC ||
         msg->type == GET_CONFIG ||
            msg->type == SET_CONFIG) {
    buffer = serialize_string(buffer, msg->param);
  }
  if(msg->buffer_size > 0) {
    buffer = serialize_int(buffer, msg->buffer_size);
    buffer = serialize_string(buffer, msg->buffer);
  }

  return buffer;
}

/** deserializers */
unsigned char *
deserialize_int(unsigned char * buffer, int * value)
{
  *value = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
  return buffer + 4;
}

unsigned char *
deserialize_char(unsigned char * buffer, char * value)
{
  *value = buffer[0];
  return buffer + 1;
}

unsigned char *
deserialize_string(unsigned char * buffer, char * str)
{
  do {
    buffer = deserialize_char(buffer, str);
  } while(*str++ != '\0');
  return buffer;
}


/**
 * Desrializes the corresponding fields for the type of the msg
 */
unsigned char *
deserialize_msg(unsigned char * buffer, msg_t * msg)
{
  unsigned char * start = buffer;
  buffer = deserialize_int(buffer, msg->bytes);
  buffer = deserialize_char(buffer, msg->type);
  if(msg->type == GET_METRIC ||
         msg->type == GET_CONFIG ||
            msg->type == SET_CONFIG) {
    buffer = deserialize_string(buffer, msg->param);
  }
  if(msg->bytes - (buffer-start) > 0) {
    buffer = deserialize_int(buffer, msg->buffer_size);
    msg->buffer = malloc(msg->buffer_size);
    buffer = deserialize_string(buffer, msg->buffer);
  }

  return buffer;
}