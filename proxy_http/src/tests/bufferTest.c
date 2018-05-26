#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "buffer/buffer.h"

static void assertReservedRead (buffer *b);
static void assertCompactLimit (buffer *b);
static void assertPeek (buffer *b);

// Tests para las funcionalidades agregadas al buffer de Juan.
int main (int argc, char *argv[]) {
	struct buffer b;
	uint8_t direct_buff[6];
	int totalSpace = 6;
	int reservedSpace = 2;
	buffer_init_r(&b, reservedSpace, totalSpace, direct_buff);

	assertReservedRead(&b);
	buffer_reset(&b);
	assertCompactLimit(&b);
	buffer_reset(&b);
	assertPeek(&b);

	return 0;
}

static void assertReservedRead (buffer *b) {
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'L');
	//            R=2 W=3
	//            ↓   ↓
	//    +---+---+---+---+---+---+
	//    |   |   | L |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write_reserved(b, 'O');
	//        R=1     W=3
	//        ↓       ↓
	//    +---+---+---+---+---+---+
	//    |   | O | L |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'A');
	//        R=1         W=4
	//        ↓           ↓
	//    +---+---+---+---+---+---+
	//    |   | O | L | A |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write_reserved(b, 'H');
	//    R=0             W=4
	//    ↓               ↓
	//    +---+---+---+---+---+---+
	//    | H | O | L | A |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write_reserved(b, 'N');
	// No escribo nada.
	assert(is_reserved(b));
	assert(buffer_read(b) == 'H');
	//        R=1         W=4
	//        ↓           ↓
	//    +---+---+---+---+---+---+
	//    | H | O | L | A |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(is_reserved(b));
	assert(buffer_read(b) == 'O');
	//            R=2     W=4
	//            ↓       ↓
	//    +---+---+---+---+---+---+
	//    | H | O | L | A |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(!is_reserved(b));
	assert(buffer_read(b) == 'L');
	//                R=3 W=4
	//                ↓   ↓
	//    +---+---+---+---+---+---+
	//    | H | O | L | A |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(!is_reserved(b));
	assert(buffer_read(b) == 'A');
	//
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
}

static void assertCompactLimit (buffer *b) {
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'H');
	//            R=2 W=3
	//            ↓   ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_read(b) == 'H');
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'H');
	//            R=2 W=3
	//            ↓   ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'O');
	//            R=2     W=4
	//            ↓       ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H | O |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_can_write(b));
	buffer_write(b, 'L');
	//            R=2         W=5
	//            ↓           ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H | O | L |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_can_write(b));
	buffer_write(b, 'A');
	//            R=2             W=6
	//            ↓               ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H | O | L | A |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(!buffer_can_write(b));
	buffer_read(b);
	buffer_read(b);
	buffer_read(b);
	assert(!buffer_can_write(b));
	buffer_read(b);
	assert(buffer_can_write(b));
}

static void assertPeek (buffer *b) {
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	buffer_write(b, 'H');
	//            R=2 W=3
	//            ↓   ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_peek(b) == 'H');
	//            R=2 W=3
	//            ↓   ↓
	//    +---+---+---+---+---+---+
	//    |   |   | H |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_read(b) == 'H');
	//            R=2/W=2
	//            ↓
	//    +---+---+---+---+---+---+
	//    |   |   |   |   |   |   |
	//    +---+---+---+---+---+---+
	//            ↑               ↑
	//            infLimit=2      limit=6
	assert(buffer_read(b) == 0);
}
