# Modulo Client

El modulo client se encarga de atender a los clientes. Internamente funciona como una maquina de estados. Requiere que se le informe de los eventos del selector. Estos eventos ejecutan distintas acciones dependiendo del estado actual del cliente. Para esto, el modulo expone cuatro handlers para cada uno de los posibles eventos sobre el file descriptor del socket cliente. Internamente tambien se usa otro file descriptor para el socket utilizado en la conexion con el origin. La maquina de estados tambien evoluciona frente a eventos en este file descriptor, pero en el contrato no se exponen los handlers pues el registro del file descriptor en el selector lo hace el modulo internamente.

## Diagrama de una conexion
```
           +---------------+   +------------------+   +----------------+
           |               |   |                  |   |                |
    +------> pre_req_parse +--->  request_parser  +---> post_req_parse +------+
    |      |   (buffer)    |   |                  |   |    (buffer)    |      |
    |      |               |   |                  |   |                |      |
    |      +---------------+   +------------------+   +----------------+      |
+--------+                                                               +----v---+
|        |                                                               |        |
| CLIENT |                                                               | ORIGIN |
|        |                                                               |        |
+---^----+                                                               +--------+
    |                                                                         |
    |     +----------------+   +-------------------+   +---------------+      |
    |     |                |   |                   |   |               |      |
    |     | post_res_parse |   |                   |   | pre_res_parse |      |
    +-----+    (buffer)    <---+  response_parser  <---+    (buffer)   <------+
          |                |   |                   |   |               |
          +----------------+   +-------------------+   +---------------+


```

##  Estados
Los estados que maneja internamente el modulo client en su maquina de estados son:

- **NO_ORIGIN:** Cuando aun no se realizo la conexion al origin.
- **SEND_REQ:** Cuando se esta escribiendo la request en el origin.
- **READ_RESP:** Cuando se esta leyendo la respuesta desde el origin.
- **ERROR:** Cuando se produjo un error.

Ademas se cuenta con dos flags que indican si ya se leyo completamente la request/response actual, estos son: `request_complete` y `response_complete`. Estos flags deberian ser seteados por el parser.

## Acciones ante eventos
Los siguientes son los posibles eventos que pueden llegar desde el selector y las acciones que toma la maquina de estados dependiendo del estado actual.

Aclaracion: cuando se pasa la request por request parser, este deberia dejar los caracteres consumidos en `post_req_parse_buffer`. Cuando se pasa la response por el response parser, este deberia dejar los caracteres consumidos en `post_res_parse_buffer`.

#### Leer del cliente
- **NO_ORIGIN:** Siempre que haya lugar en el buffer, pasamos la request por el request parser. Cuando este obtenga el host, deberia llamar al callback para setear el host, iniciandose la resolucion del nombre y posteriormente, la conexion.
- **SEND_REQ:** Pasamos la request por el request parser.

En el resto de los estados no hacemos nada ante este evento.

#### Escribir hacia el cliente
- **READ_RES:** Si hay caracteres en `post_res_parse_buffer`, los enviamos al cliente. Si no hay caracteres en `post_res_parse_buffer` y esta seteado `response_complete` pasamos al estado **READ_REQ** (reiniciando el estado de los flags).
- **NO_REMOTE:** Si hay caracteres en el buffer de salida, los enviamos al cliente. Si no hay caracteres en el buffer de salida liberamos los recursos y hacemos close del cliente.
- **ERROR:** Chequeamos que error se produjo, y si es necesario, se envia al cliente una response acorde.

En el resto de los estados no hacemos nada ante este evento.

#### Desbloqueo en el cliente
- **HOST_RESOLV:** Hacemos connect al origin y pasamos al estado **SEND_REQ**.

En el resto de los estados no hacemos nada ante este evento.

#### Close en el cliente
Liberamos los recursos del cliente (buffers, parser, socket al origin, fd's, etc.).

#### Leer en origin
- **READ_RESP:** Pasamos la response por el response parser.

#### Escribir en origin
- **SEND_REQ:** Si hay caracteres en `post_req_parse_buffer`, los enviamos a origin. Si no hay caracteres en `post_req_parse_buffer` y esta seteado `request_complete` pasamos al estado **READ_RESP**.

#### Close en origin
- **READ_RESP:** Desregistramos al origin del selector y vamos al estado **NO_ORIGIN**.

## Implementacion
Aca se detallan notas de la implementacion

#### TO DO:
 - Falta chequear errores
 - Falta revisar como responder errores (tal vez se necesita agregar un estado "**SENDING_ERROR**")
 - Falta acoplar los parsers (actualmente solo funciona con los parsers dummy)
 - Crear los buffers sacando los tamaños del modulo Config.
 - Restaurar el estado del cliente (`client_restart_state`)
 - Liberar recursos (`client_free_resources`)
 - Intentar conectar con todas las ip's obtenidas
