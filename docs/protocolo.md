# Protocolo

Las tareas que necesitamos son:
- Autenticar (enviar credenciales)
- Listar métricas
- Listar configuraciones
- Pedir métricas
- Pedir configuraciones
- Setear configuraciones

## Resumen
Definimos entonces, un protocolo binario, orientado a conexión, de tipo *request/response*, donde en la request se envía la tarea que se quiere realizar y en la response el resultado de ésta.
Por tratarse de un protocolo binario, definimos también las correspondientes funciones de serialización.

## Estructura de un mensaje
- **type**: 1 Byte que representa la acción.
  - 0: Enviar credenciales (pass-phrase)
  - 1: Listar métricas
  - 2: Listar configuraciones
  - 3: Pedir métricas
  - 4: Pedir configuraciones
  - 5: Setear configuraciones
- **param**: 1 byte que indica (de ser necesario) de qué metrica/configuracion se trata. El número representado por el byte será el número de métrica/configuración de sus respectivas listas.
- **buffer_size**: 4 bytes que indican (de existir un mensaje) el tamaño del mensaje que sigue.
- **buffer**: mensaje del tamaño de message_size.
### Los mensajes serán:
- Enviar credenciales
- La lista de métricas
- La lista de configuraciones
- El valor de una métrica
- El valor de una configuración
- Indicar error

## Breve descripcion de la implementación
El protocolo contará con la siguiente definición de constantes:
```C
SEND_CREDENTIALS  0
LIST_METRICS      1
LIST_CONFIGS      2
GET_METRIC        3
GET_CONFIG        4
SET_CONFIG        5
ERROR             6
```
En C, un mensaje será representado por una estructura msg_t:
```C
typedef struct {
    unsigned char   type;
    unsigned char   param;
    unsigned int    buffer_size;
    unsigned char * buffer;
} msg_t;
```
El único campos que siempre estará inicializados será *type*. A éste se les agregan los demás dependiendo del tipo de mensaje:
- **SEND_CREDENTIALS**: Si se está del lado del administrador: *buffer_size* y *buffer*.
- **LIST_METRCIS y LIST_CONFIGS**: Si se está del lado del proxy: *buffer_size* y *buffer*.
- **GET_METRIC y GET_CONFIG**: *param* y si se está del lado del proxy: *buffer_size* y *buffer*.
- **SET_CONFIG**: Si se está del lado del administrador: *param*, *buffer_size* y *buffer*.


## DANI COMENTS
Cambiar messages.h: que se envien msgs serializados.
msgs_t lo tengo que usar en el offer nada mas: lo serializo en el offer y dps lo meto en la msg_queue.
El send msg tiene que simplemente mandar los bytes que pueda y retornar la cantidad que mando.
Desde el lado del msg_queue tengo que ver si todavia no se mando todo, lo PUSHEO a la cola para que luego se mande lo que falta de ese mensaje primero.
La parte de recieve si la maneja con msg_t
