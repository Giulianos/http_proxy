#include <protocol/protocol.h>
#include <serializer/serializer.h>
#include <stdlib.h>

/** serializers */

unsigned char*
serialize_int(unsigned char* buffer, int value)
{
  buffer[0] = (unsigned char)(value >> 24 & 0xFF);
  buffer[1] = (unsigned char)(value >> 16 & 0xFF);
  buffer[2] = (unsigned char)(value >> 8 & 0xFF);
  buffer[3] = (unsigned char)(value & 0xFF);
  return buffer + 4;
}

unsigned char*
serialize_char(unsigned char* buffer, unsigned char value)
{
  buffer[0] = value;
  return buffer + 1;
}

unsigned char*
serialize_string(unsigned char* buffer, unsigned char* str)
{
  do {
    buffer = serialize_char(buffer, *str);
  } while (*str++ != '\0');

  return buffer;
}
/**
 * Serializes the corresponding fields for the type of the msg
 */
unsigned char*
serialize_msg(unsigned char* buffer, msg_t* msg)
{
  /** type serialization */
  buffer = serialize_char(buffer, msg->type);
  /** param serialization */
  buffer = serialize_char(buffer, msg->param);
  /** buffer size serialization */
  buffer = serialize_int(buffer, msg->buffer_size);
  /** buffer serialization if present */
  if (msg->buffer_size > 0) {
    buffer = serialize_string(buffer, msg->buffer);
  }

  return buffer;
}

/** deserializers */
unsigned char*
deserialize_int(unsigned char* buffer, int* value)
{
  *value = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
  return buffer + 4;
}

unsigned char*
deserialize_char(unsigned char* buffer, unsigned char* value)
{
  *value = buffer[0];
  return buffer + 1;
}

unsigned char*
deserialize_string(unsigned char* buffer, unsigned char* str)
{
  do {
    buffer = deserialize_char(buffer, str);
  } while (*str++ != '\0');
  return buffer;
}

/**
 * Desrializes the corresponding fields for the type of the msg
 */
unsigned char*
deserialize_msg(unsigned char* buffer, msg_t* msg)
{
  /** deserialization of type */
  buffer = deserialize_char(buffer, &msg->type);
  /** param deserialization */
  buffer = deserialize_char(buffer, &msg->param);
  /** buffer size deserialization */
  buffer = deserialize_int(buffer, &msg->buffer_size);

  if (msg->buffer_size > 0) {
    msg->buffer = malloc((size_t)msg->buffer_size);
    if (msg->buffer == NULL)
      return NULL;
    buffer = deserialize_string(buffer, msg->buffer);
  }

  return buffer;
}
