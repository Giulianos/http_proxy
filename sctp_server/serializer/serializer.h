#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <protocol/protocol.h>


/**
* SERIALIZERS
* Serializes the corresponding type so that it can be sent through the socket, ensuring a good arrival.
* @param buffer with space to save what gets serialized
* @param value to be serialized
* @return the buffer's direction after serializing, so that the complete message gets serialized in contiguous memory
*/

unsigned char *
serialize_int(unsigned char * buffer, int value);
unsigned char *
serialize_char(unsigned char * buffer, char value);
unsigned char *
serialize_string(unsigned char * buffer, char * str);
unsigned char *
serialize_msg(unsigned char * buffer, msg_t * msg);


/**
 * DESERIALIZERS
 * Deserializes the corresponding type so that it can be re built after reading from the socket, ensuring a good reception.
 * @param buffer containing what needs to be deserialized
 * @param value where it saves the deserialized value
 * @return the buffer's direction after serializing, so that the message can continue to deserialize
 */

unsigned char *
deserialize_int(unsigned char * buffer, int * value);
unsigned char *
deserialize_char(unsigned char * buffer, unsigned char * value);
unsigned char *
deserialize_string(unsigned char * buffer, unsigned char * str);
unsigned char *
deserialize_msg(unsigned char * buffer, msg_t * msg);

#endif



