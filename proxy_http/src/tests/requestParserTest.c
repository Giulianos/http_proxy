#include "../request_parser/requestParser.c" // Source code included to test static functions.
#include <assert.h>

static void assertCompleteMethod(RequestData* rData, buffer* b, buffer* bOut);
static void assertIncompleteMethod(RequestData* rData, buffer* b, buffer* bOut);
static void assertCompleteUriHost(RequestData* rData, buffer* b, buffer* bOut);
static void assertIncompleteUriHost(RequestData* rData, buffer* b,
                                    buffer* bOut);
static void assertCompleteHost(RequestData* rData, buffer* b, buffer* bOut);
static void assertIncompleteHost(RequestData* rData, buffer* b, buffer* bOut);
static void assertVersion(RequestData* rData, buffer* b, buffer* bOut);
static void assertUri(RequestData* rData, buffer* b, buffer* bOut);
static void assertLocalHost(RequestData* rData, buffer* b, buffer* bOut);
static void assertHostHeader(RequestData* rData, buffer* b, buffer* bOut);
static void assertCompleteRequest(RequestData* rData, buffer* b, buffer* bOut);
static void assertIncompleteRequestWithHost(RequestData* rData, buffer* b,
                                            buffer* bOut);
static void assertIncompleteRequestWithUriHost(RequestData* rData, buffer* b,
                                               buffer* bOut);
static void assertIncompleteRequestWithHostByByte(RequestData* rData, buffer* b,
                                                  buffer* bOut);
static void assertIncompleteRequestWithUriHostByByte(RequestData* rData,
                                                     buffer* b, buffer* bOut);
static void insertToBuffer(RequestData* rData, char* text, buffer* b,
                           buffer* bOut);
static void resetData(RequestData* rData, buffer* b, buffer* bOut);

int
main(int argc, char* argv[])
{
  RequestData* rData = (RequestData*)malloc(sizeof(RequestData));
  struct buffer b;
  uint8_t direct_buff[100];
  int totalSpace = 100;
  int reservedSpace = 20;
  struct buffer bOut;
  uint8_t direct_buff_out[100];
  int totalSpaceOut = 100;
  buffer_init_r(&b, reservedSpace, totalSpace, direct_buff);
  buffer_init(&bOut, totalSpaceOut, direct_buff_out);

  if (rData == NULL) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return 1;
  }

  assertCompleteMethod(rData, &b, &bOut);
  assertIncompleteMethod(rData, &b, &bOut);
  assertCompleteUriHost(rData, &b, &bOut);
  assertIncompleteUriHost(rData, &b, &bOut);
  assertCompleteHost(rData, &b, &bOut);
  assertIncompleteHost(rData, &b, &bOut);
  assertVersion(rData, &b, &bOut);
  assertUri(rData, &b, &bOut);
  assertLocalHost(rData, &b, &bOut);
  assertHostHeader(rData, &b, &bOut);
  assertCompleteRequest(rData, &b, &bOut);
  assertIncompleteRequestWithHost(rData, &b, &bOut);
  assertIncompleteRequestWithUriHost(rData, &b, &bOut);
  assertIncompleteRequestWithHostByByte(rData, &b, &bOut);
  assertIncompleteRequestWithUriHostByByte(rData, &b, &bOut);

  free(rData);

  return 0;
}

static void
assertCompleteMethod(RequestData* rData, buffer* b, buffer* bOut)
{
  // Métodos soportados.
  insertToBuffer(rData, "Get", b, bOut);
  assert(extractHttpMethod(rData, b, bOut));
  assert(rData->method == GET);

  insertToBuffer(rData, "HeaD", b, bOut);
  assert(extractHttpMethod(rData, b, bOut));
  assert(rData->method == HEAD);

  insertToBuffer(rData, "POST", b, bOut);
  assert(extractHttpMethod(rData, b, bOut));
  assert(rData->method == POST);

  // Método no soportado.
  insertToBuffer(rData, "PUT", b, bOut);
  assert(!extractHttpMethod(rData, b, bOut));
  assert(rData->method == PUT);
}

static void
assertIncompleteMethod(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "HeA", b, bOut);
  assert(!extractHttpMethod(rData, b, bOut));
  assert(rData->method == UNDEFINED_M);
  assert(rData->isBufferEmpty);
  buffer_write(b, 'd');
  assert(extractHttpMethod(rData, b, bOut));
  assert(rData->method == HEAD);
}

static void
assertCompleteUriHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "example.org/", b, bOut);
  assert(checkUriForHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);

  insertToBuffer(rData, "userinfo@example.org/", b, bOut);
  assert(checkUriForHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);

  insertToBuffer(rData, "userinfo@example.org:8080/", b, bOut);
  assert(checkUriForHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertIncompleteUriHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "example.o", b, bOut);
  assert(!checkUriForHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) != 0);
  writeToBuf("rg/", b); // Con el '/' indico que termino de leer el host

  assert(rData->port == 80);
  insertToBuffer(rData, "example.org:", b, bOut);
  assert(!checkUriForHost(rData, b, bOut));
  buffer_write(b, '9');
  assert(!checkUriForHost(rData, b, bOut));
  assert(rData->port == 80);
  // Todavía no cambién el puerto porque no se si hay algo después del 9.
  buffer_write(b, ' ');
  assert(checkUriForHost(rData, b, bOut));
  assert(rData->port == 9);
}

static void
assertCompleteHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "example.org\r", b, bOut);
  assert(extractHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);

  // Puerto mal formado.
  insertToBuffer(rData, "example2.org:h8080\r", b, bOut);
  assert(!extractHost(rData, b, bOut));

  insertToBuffer(rData, "example2.org:8080\r", b, bOut);
  assert(extractHost(rData, b, bOut));
  assert(strcmp("example2.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertIncompleteHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "example.o", b, bOut);
  assert(!extractHost(rData, b, bOut));
  assert(strcmp("example.org", rData->host) != 0);
  writeToBuf("rg\r", b); // Con el '\r' indico que termino de leer el host

  assert(rData->port == 80);
  insertToBuffer(rData, "example.org:", b, bOut);
  assert(!extractHost(rData, b, bOut));
  buffer_write(b, '9');
  assert(!extractHost(rData, b, bOut));
  assert(rData->port == 80);
  // Todavía no cambién el puerto porque no se si hay algo después del 9.
  buffer_write(b, ' ');
  assert(extractHost(rData, b, bOut));
  assert(rData->port == 9);
}

static void
assertVersion(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "hTt", b, bOut);
  assert(!extractHttpVersion(rData, b, bOut));
  writeToBuf("p/1.", b);
  assert(!extractHttpVersion(rData, b, bOut));
  buffer_write(b, '1');
  assert(rData->version == UNDEFINED);
  assert(extractHttpVersion(rData, b, bOut));
  assert(rData->version == V_1_1);
}

static void
assertUri(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "ht", b, bOut);
  assert(!checkUri(rData, b, bOut));
  assert(PEEK_UP_CHAR(b) == 'H');
  writeToBuf("tp", b);
  assert(!checkUri(rData, b, bOut));
  assert(PEEK_UP_CHAR(b) == 'H');
  writeToBuf("://", b);
  assert(!checkUri(rData, b, bOut));
  assert(buffer_peek(b) == 0);
  // Cuando pasé el // voy a la función checkUriForHost
  // por lo que no guardo el http://

  writeToBuf("http://userinfo@example.org/", b);
  assert(checkUri(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);

  insertToBuffer(rData, "http://userinfo@example.org:8080/", b, bOut);
  assert(checkUri(rData, b, bOut));
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertLocalHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "Test:\r\n", b, bOut);
  assert(!checkLocalHost(rData, b, bOut));
  assert(!rData->isLocalHost);

  insertToBuffer(rData, "X-LOCaL", b, bOut);
  assert(!checkLocalHost(rData, b, bOut));
  assert(!rData->isLocalHost);
  writeToBuf("HOST:\r\n", b);
  assert(checkLocalHost(rData, b, bOut));
  assert(rData->isLocalHost);
}

static void
assertHostHeader(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "gdgd\r\n\r", b, bOut);
  assert(!checkHostHeader(rData, b, bOut));
  assert(rData->isBufferEmpty);
  writeToBuf("\n", b);
  // Paso a tener fin de headers - \r\n\r\n
  assert(buffer_peek(b) == '\r');
  assert(!checkHostHeader(rData, b, bOut));
  assert(buffer_peek(b) == 0);

  insertToBuffer(rData, "Hos", b, bOut);
  assert(!checkHostHeader(rData, b, bOut));
  assert(PEEK_UP_CHAR(b) == 'H');
  writeToBuf("t:", b);
  assert(checkHostHeader(rData, b, bOut));
  assert(PEEK_UP_CHAR(b) == 0);
  // Cuando pasé el Host: voy a la función extractHost
  // por lo que no guardo el Host:
}

static void
assertCompleteRequest(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "gEt /foo HTtP/1.1\r\nHost: example.org:8080\r\n", b,
                 bOut);
  assert(checkRequestInner(rData, b, bOut));
  assert(rData->method == GET);
  assert(rData->version == V_1_1);
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertIncompleteRequestWithHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "gE", b, bOut);
  assert(rData->parserState == METHOD);
  assert(!checkRequestInner(rData, b, bOut));
  writeToBuf("t", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->method == GET);
  assert(rData->parserState == SPACE_TRANSITION);
  assert(rData->next == URI);
  writeToBuf(" /fo", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == RELATIVE_URI);
  writeToBuf("o HTtP/1.", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == VERSION);
  writeToBuf("1", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->version == V_1_1);
  assert(rData->parserState == START_LINE_END);
  writeToBuf("\r", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == START_LINE_END);
  writeToBuf("\n", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == LOCALHOST_HEADER_CHECK);
  writeToBuf("Host", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == HEADERS);
  writeToBuf(":", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == SPACE_TRANSITION);
  assert(rData->next == HOST);
  writeToBuf(" example.org", b);
  assert(!checkRequestInner(rData, b, bOut));
  writeToBuf(":8080\r", b);
  assert(checkRequestInner(rData, b, bOut));
  assert(rData->parserState == FINISHED);
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertIncompleteRequestWithUriHost(RequestData* rData, buffer* b, buffer* bOut)
{
  insertToBuffer(rData, "gEt http:/", b, bOut);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == URI);
  writeToBuf("/user", b);
  assert(!checkRequestInner(rData, b, bOut));
  assert(rData->parserState == URI_HOST);
  writeToBuf("info@example.org:8080/", b);
  assert(checkRequestInner(rData, b, bOut));
  assert(rData->parserState == FINISHED);
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);
}

static void
assertIncompleteRequestWithHostByByte(RequestData* rData, buffer* b,
                                      buffer* bOut)
{
  char* msg = "gEt \t /foo HTtP/1.1\r\nHost: example.org:8080";
  char* msgOut =
    "gEt /foo HTtP/1.1\r\nX-LOCALHOST: TRUE\r\nHost: example.org:8080";
  char aux[2] = { 0 };
  insertToBuffer(rData, "", b, bOut);
  int i = 0;
  while (msg[i] != 0) {
    aux[0] = msg[i];
    writeToBuf(aux, b);
    assert(!checkRequestInner(rData, b, bOut));
    i++;
  }
  writeToBuf("\r", b); // Pongo un delimitador de fin de host.
  assert(checkRequestInner(rData, b, bOut));
  assert(rData->parserState == FINISHED);
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);

  // Veo que en la salida queda lo que espero
  for (int i = 0; msgOut[i] != 0; i++) {
    assert(msgOut[i] == buffer_read(bOut));
  }
  assert(!buffer_can_read(bOut));
}

static void
assertIncompleteRequestWithUriHostByByte(RequestData* rData, buffer* b,
                                         buffer* bOut)
{
  char* msg = "gEt http://userinfo@example.org:8080";
  char aux[2] = { 0 };
  insertToBuffer(rData, "", b, bOut);
  int i = 0;
  while (msg[i] != 0) {
    aux[0] = msg[i];
    writeToBuf(aux, b);
    assert(!checkRequestInner(rData, b, bOut));
    i++;
  }
  writeToBuf("/", b); // Pongo un delimitador de fin de host.
  assert(checkRequestInner(rData, b, bOut));
  assert(rData->parserState == FINISHED);
  assert(strcmp("example.org", rData->host) == 0);
  assert(rData->port == 8080);

  // Veo que en la salida queda lo que espero
  for (int i = 0; msg[i] != 0; i++) {
    assert(msg[i] == buffer_read(bOut));
  }
  assert(!buffer_can_read(bOut));
}

static void
insertToBuffer(RequestData* rData, char* text, buffer* b, buffer* bOut)
{
  int i = 0;

  resetData(rData, b, bOut);

  while (text[i] != 0) {
    buffer_write(b, text[i]);
    i++;
  }
}

static void
resetData(RequestData* rData, buffer* b, buffer* bOut)
{
  buffer_reset(b);
  buffer_reset(bOut);
  rData->parserState = METHOD;
  rData->isBufferEmpty = false;
  rData->state = OK;
  rData->version = UNDEFINED;
  rData->method = UNDEFINED_M;
  rData->port = DEFAULT_PORT;
  rData->isLocalHost = false;

  for (int i = 0; i < HOST_MAX_SIZE && rData->host[i] != 0; i++) {
    rData->host[i] = 0;
  }
}
