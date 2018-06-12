#include "../response_parser/responseParser.c" // Source code included to test static functions.
#include <assert.h>

static void assertVersion(ResponseData* rData, buffer* b, buffer* bOut);
static void assertStatus(ResponseData* rData, buffer* b, buffer* bOut);
static void assertCompleteResponseWithLength(ResponseData* rData, buffer* b,
                                             buffer* bOut, buffer* bTransf);
static void assertConnection(ResponseData* rData, buffer* b, buffer* bOut,
                             buffer* bTransf);
static void assertCompleteResponseWithLengthAndTransf(ResponseData* rData,
                                                      buffer* b, buffer* bOut,
                                                      buffer* bTransf);
static void assertCompleteResponseWithChunk(ResponseData* rData, buffer* b,
                                            buffer* bOut, buffer* bTransf);
static void assertCompleteResponseWithChunkAndZeros(ResponseData* rData,
                                                    buffer* b, buffer* bOut,
                                                    buffer* bTransf);
static void assertCompleteResponseWithChunkAndTransf(ResponseData* rData,
                                                     buffer* b, buffer* bOut,
                                                     buffer* bTransf);
static void assertIncompleteResponseWithChunk(ResponseData* rData, buffer* b,
                                              buffer* bOut, buffer* bTransf);
static void assertIncompleteResponseWithChunkAndTransf(ResponseData* rData,
                                                       buffer* b, buffer* bOut,
                                                       buffer* bTransf);
static void assertIncompleteResponseWithLengthByByte(ResponseData* rData,
                                                     buffer* b, buffer* bOut,
                                                     buffer* bTransf);
static void assertIncompleteResponseWithLengthByByteAndZeros(
  ResponseData* rData, buffer* b, buffer* bOut, buffer* bTransf);
static void assertIncompleteResponseWithChunkByByte(ResponseData* rData,
                                                    buffer* b, buffer* bOut,
                                                    buffer* bTransf);
static void assertIncompleteResponseWithChunkByByteAndZeros(ResponseData* rData,
                                                            buffer* b,
                                                            buffer* bOut,
                                                            buffer* bTransf);
static void assertIncompleteResponseWithChunkAndTransfByByte(
  ResponseData* rData, buffer* b, buffer* bOut, buffer* bTransf);
static void insertToBuffer(ResponseData* rData, char* text, buffer* b,
                           buffer* bOut);
static void insertToBufferWithZeros(ResponseData* rData, char* text, buffer* b,
                                    buffer* bOut);
static void resetData(ResponseData* rData, buffer* b, buffer* bOut);

int
main(int argc, char* argv[])
{
  ResponseData* rData = (ResponseData*)malloc(sizeof(ResponseData));
  struct buffer b;
  uint8_t direct_buff[200];
  int totalSpace = 200;
  int reservedSpace = 20;
  struct buffer bOut;
  uint8_t direct_buff_out[200];
  int totalSpaceOut = 200;
  struct buffer bTransf;
  uint8_t direct_buff_transf[200];
  int totalSpaceTransf = 200;
  buffer_init_r(&b, reservedSpace, totalSpace, direct_buff);
  buffer_init(&bOut, totalSpaceOut, direct_buff_out);
  buffer_init(&bTransf, totalSpaceTransf, direct_buff_transf);

  if (rData == NULL) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return 1;
  }

  assertVersion(rData, &b, &bOut);
  assertStatus(rData, &b, &bOut);
  assertCompleteResponseWithLength(rData, &b, &bOut, &bTransf);
  assertConnection(rData, &b, &bOut, &bTransf);
  assertCompleteResponseWithLengthAndTransf(rData, &b, &bOut, &bTransf);
  assertCompleteResponseWithChunk(rData, &b, &bOut, &bTransf);
  assertCompleteResponseWithChunkAndZeros(rData, &b, &bOut, &bTransf);
  assertCompleteResponseWithChunkAndTransf(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithChunk(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithChunkAndTransf(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithLengthByByte(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithLengthByByteAndZeros(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithChunkByByte(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithChunkByByteAndZeros(rData, &b, &bOut, &bTransf);
  assertIncompleteResponseWithChunkAndTransfByByte(rData, &b, &bOut, &bTransf);

  free(rData);

  return 0;
}

static void
assertVersion(ResponseData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "hTt", b, bOut);
  assert(!extractHttpVersion(rData, b, bOut));
  writeToBuf("p/1.", b);
  assert(!extractHttpVersion(rData, b, bOut));
  buffer_write(b, '1');
  assert(rData->version == UNDEFINED);
  assert(extractHttpVersion(rData, b, bOut));
  assert(rData->version == V_1_1);

  insertToBuffer(rData, "hTtp/1.2", b, bOut); // Versión incorrecta.
  assert(!extractHttpVersion(rData, b, bOut));
  assert(rData->version == UNDEFINED);
}

static void
assertStatus(ResponseData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "d200", b, bOut);
  assert(!extractStatus(rData, b, bOut));
  buffer_read(b); // Saco la d.
  assert(!extractStatus(rData, b,
                        bOut)); // No se si tengo otro número después del 0.
  buffer_write(b, ' ');
  assert(extractStatus(rData, b, bOut));
  assert(rData->status == 200);

  // insertToBuffer(rData, "400 ", b, bOut);
  // assert(!extractStatus(rData, b, bOut));
  // assert(rData->status == 400);
}

static void
assertCompleteResponseWithLength(ResponseData* rData, buffer* b, buffer* bOut,
                                 buffer* bTransf)
{
  char* msgIn = "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWikipedia";
  char* msgOut =
    "HTTP/1.1 200 OK\r\nconTeNt-lengTh: 3\r\nConnection: close\r\n\r\nWik";
  insertToBuffer(rData, msgIn, b, bOut);
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->bodyLength == 0);    // Terminé de leer todo.
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.

  for (int i = 0; msgOut[i] != 0; i++) {
    assert(buffer_read(bOut) == msgOut[i]);
  }
  assert(buffer_read(bOut) == 0);
}

static void
assertConnection(ResponseData* rData, buffer* b, buffer* bOut, buffer* bTransf)
{
  char* msgIn =
    "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nconTeNt-lengTh:3\r\n\r\nWik";
  char* msgOut =
    "HTTP/1.1 200 OK\r\nConnection: close\r\nconTeNt-lengTh: 3\r\n\r\nWik";
  insertToBuffer(rData, msgIn, b, bOut);
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);

  for (int i = 0; msgOut[i] != 0; i++) {
    assert(buffer_read(bOut) == msgOut[i]);
  }
  assert(buffer_read(bOut) == 0);
}

static void
assertCompleteResponseWithLengthAndTransf(ResponseData* rData, buffer* b,
                                          buffer* bOut, buffer* bTransf)
{
  insertToBuffer(rData, "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWikipedia",
                 b, bOut);
  rData->withTransf = true;
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->bodyLength == 0); // Terminé de leer todo.
  assert(buffer_read(bTransf) == 'W');
  assert(buffer_read(bTransf) == 'i');
  assert(buffer_read(bTransf) == 'k');
  assert(buffer_read(bTransf) == 0);
}

static void
assertCompleteResponseWithChunk(ResponseData* rData, buffer* b, buffer* bOut,
                                buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 "
              "OK\r\ntransfer-encoDing:chunkEd\r\n\r\n11\r\n0123456789 "
              "Wikipe\r\n3\r\ndia\r\n0\r\n\r\n";
  char* msgOut = "HTTP/1.1 200 OK\r\ntransfer-encoDing: chunkEd\r\nConnection: "
                 "close\r\n\r\n11\r\n0123456789 "
                 "Wikipe\r\n3\r\ndia\r\n0\r\n\r\n";
  insertToBuffer(rData, msg, b, bOut);
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->isChunked);
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.

  for (int i = 0; msgOut[i] != 0; i++) {
    assert(buffer_read(bOut) == msgOut[i]);
  }
  assert(buffer_read(bOut) == 0);
}

static void
assertCompleteResponseWithChunkAndZeros(ResponseData* rData, buffer* b,
                                        buffer* bOut, buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 "
              "OK\r\ntransfer-encoDing:chunkEd\r\n\r\n11\r\n01\0l4\0k789 "
              "Wikipe\r\n3\r\ndia\r\n0\r\n\r\n\1";
  char* msgOut = "HTTP/1.1 200 OK\r\ntransfer-encoDing: chunkEd\r\nConnection: "
                 "close\r\n\r\n11\r\n01\0l4\0k789 "
                 "Wikipe\r\n3\r\ndia\r\n0\r\n\r\n\1";
  int i = 0;

  insertToBufferWithZeros(rData, msg, b, bOut);

  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->isChunked);
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.

  while (buffer_can_read(bOut)) {
    assert(buffer_read(bOut) == msgOut[i]);
    i++;
  }
  assert(msgOut[i] == '\1');
}

static void
assertCompleteResponseWithChunkAndTransf(ResponseData* rData, buffer* b,
                                         buffer* bOut, buffer* bTransf)
{
  char* msg = "Wikipedia";
  insertToBuffer(rData, "HTTP/1.1 200 "
                        "OK\r\ntransfer-encoDing:"
                        "chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n",
                 b, bOut);
  rData->withTransf = true;
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->isChunked);

  // Al buffer de la transformación espero haber paso solo el contenido de los
  // chunks.
  for (int i = 0; msg[i] != 0; i++) {
    assert(buffer_read(bTransf) == msg[i]);
  }
  assert(buffer_read(bTransf) == 0);
}

static void
assertIncompleteResponseWithChunk(ResponseData* rData, buffer* b, buffer* bOut,
                                  buffer* bTransf)
{
  insertToBuffer(rData, "HTTP/1.", b, bOut);
  assert(rData->parserState == RES_VERSION);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  writeToBuf("1 20", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->version == V_1_1);
  assert(rData->parserState == STATUS);
  writeToBuf("0 OK\r\ntransfer-enc", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->status == 200);
  assert(rData->parserState == RES_HEADERS);
  writeToBuf("oDing:c", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == CHUNKED_CHECK);
  writeToBuf("hunkEd\r\n\r\n", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->isChunked);
  assert(rData->parserState == BODY_NORMAL);
  writeToBuf("4\r\nWi", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  writeToBuf("ki\r\n5\r\npedia\r\n0\r\n\r\n", b);
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
}

static void
assertIncompleteResponseWithChunkAndTransf(ResponseData* rData, buffer* b,
                                           buffer* bOut, buffer* bTransf)
{
  char* msg = "Wikipedia";
  insertToBuffer(rData, "HTTP/1.1 200 OK\r\ntransfer-encoDing:chunkEd\r\n\r\n",
                 b, bOut);
  rData->withTransf = true;
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->isChunked);
  assert(rData->parserState == BODY_TRANSFORMATION);
  writeToBuf("4\r\nWi", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  writeToBuf("ki\r\n5\r\npedia\r", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  writeToBuf("\n0\r\n\r", b);
  assert(!checkResponseInner(rData, b, bOut, bTransf));
  writeToBuf("\n", b);
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);

  // Al buffer de la transformación espero haber paso solo el contenido de los
  // chunks.
  for (int i = 0; msg[i] != 0; i++) {
    assert(buffer_read(bTransf) == msg[i]);
  }
  assert(buffer_read(bTransf) == 0);
}

static void
assertIncompleteResponseWithLengthByByte(ResponseData* rData, buffer* b,
                                         buffer* bOut, buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 OK\r\nconTeNt-lengTh:3\r\n\r\nWik";
  resetData(rData, b, bOut);
  int i = 0;
  while (msg[i] != 0) {
    // Hago la comparación del paso anterior. Esto lo hago para que no rompa en
    // el último paso.
    assert(!checkResponseInner(rData, b, bOut, bTransf));
    buffer_write(b, msg[i]);
    i++;
  }
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->bodyLength == 0);    // Terminé de leer todo.
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertIncompleteResponseWithLengthByByteAndZeros(ResponseData* rData, buffer* b,
                                                 buffer* bOut, buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 OK\r\nconTeNt-lengTh:5\r\n\r\nW\0i\0k\1";
  char* msgOut = "HTTP/1.1 200 OK\r\nconTeNt-lengTh: 5\r\nConnection: "
                 "close\r\n\r\nW\0i\0k\1";

  resetData(rData, b, bOut);
  int i = 0;
  while (msg[i] != '\1') { // Como tengo 0s en msg pongo un 1 como delimitador.
    // Hago la comparación del paso anterior. Esto lo hago para que no rompa en
    // el último paso.
    assert(!checkResponseInner(rData, b, bOut, bTransf));
    buffer_write(b, msg[i]);
    i++;
  }
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->bodyLength == 0); // Terminé de leer todo.

  i = 0;
  while (buffer_can_read(bOut)) {
    assert(buffer_read(bOut) == msgOut[i]);
    i++;
  }
}

static void
assertIncompleteResponseWithChunkByByte(ResponseData* rData, buffer* b,
                                        buffer* bOut, buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 OK\r\ntransfer-encoDing: "
              "chunkEd\r\n\r\n11\r\n0123456789 Wikipe\r\n3\r\ndia\r\n0\r\n\r\n";
  resetData(rData, b, bOut);
  int i = 0;
  while (msg[i] != 0) {
    // Hago la comparación del paso anterior. Esto lo hago para que no rompa en
    // el último paso.
    assert(!checkResponseInner(rData, b, bOut, bTransf));
    buffer_write(b, msg[i]);
    i++;
  }
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->isChunked);
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertIncompleteResponseWithChunkByByteAndZeros(ResponseData* rData, buffer* b,
                                                buffer* bOut, buffer* bTransf)
{
  char* msg = "HTTP/1.1 200 OK\r\ntransfer-encoDing:chunkEd\r\n\r\n9\r\nH\0la "
              "Wiki\r\n5\r\npedia\r\n0\r\n\r\n\1";
  resetData(rData, b, bOut);
  int i = 0;
  while (msg[i] != '\1') { // Como tengo 0s en msg pongo un 1 como delimitador.
    // Hago la comparación del paso anterior. Esto lo hago para que no rompa en
    // el último paso.
    assert(!checkResponseInner(rData, b, bOut, bTransf));
    buffer_write(b, msg[i]);
    i++;
  }
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);
  assert(rData->version == V_1_1);
  assert(rData->status == 200);
  assert(rData->isChunked);
  assert(buffer_read(bTransf) == 0); // Transformaciones apagadas.
}

static void
assertIncompleteResponseWithChunkAndTransfByByte(ResponseData* rData, buffer* b,
                                                 buffer* bOut, buffer* bTransf)
{
  char* msgTransf = "Wikipedia";
  char* msg = "HTTP/1.1 200 "
              "OK\r\ntransfer-encoDing:"
              "chunkEd\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n";
  resetData(rData, b, bOut);
  rData->withTransf = true;
  int i = 0;
  while (msg[i] != 0) {
    // Hago la comparación del paso anterior. Esto lo hago para que no rompa en
    // el último paso.
    assert(!checkResponseInner(rData, b, bOut, bTransf));
    buffer_write(b, msg[i]);
    i++;
  }
  assert(checkResponseInner(rData, b, bOut, bTransf));
  assert(rData->parserState == RES_FINISHED);

  // Al buffer de la transformación espero haber paso solo el contenido de los
  // chunks.
  for (int i = 0; msgTransf[i] != 0; i++) {
    assert(buffer_read(bTransf) == msgTransf[i]);
  }
  assert(buffer_read(bTransf) == 0);
}

static void
insertToBuffer(ResponseData* rData, char* text, buffer* b, buffer* bOut)
{
  int i = 0;

  resetData(rData, b, bOut);

  while (text[i] != 0) {
    buffer_write(b, text[i]);
    i++;
  }
}

static void
insertToBufferWithZeros(ResponseData* rData, char* text, buffer* b,
                        buffer* bOut)
{
  int i = 0;

  resetData(rData, b, bOut);

  while (text[i] != '\1') { // Delimitador que uso para diferenciar del 0.
    buffer_write(b, text[i]);
    i++;
  }
}

static void
resetData(ResponseData* rData, buffer* b, buffer* bOut)
{
  buffer_reset(b);
  buffer_reset(bOut);
  rData->parserState = RES_VERSION;
  rData->isBufferEmpty = false;
  rData->state = RES_OK;
  rData->version = UNDEFINED;
  rData->status = 0;
  rData->bodyLength = NO_BODY_LENGTH;
  rData->cEncoding = IDENTITY;
  rData->isClose = false;
  rData->isChunked = false;
  rData->withTransf = false;
}
