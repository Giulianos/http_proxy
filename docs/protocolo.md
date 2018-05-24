# Protocolo

Las tareas que necesitamos son:
- Listar metricas
- Listar configuraciones
- Pedir metricas
- Pedir configuraciones
- Setear configuraciones

## Resumen
Definimos entonces, un protocolo binario, orientado a conexion, de tipo *request/response*, donde en la request se envia la tarea que se quiere realizar y en la response el resultado de esta.
Por tratarse de un protocolo binario, definimos tambien las correspondientes funciones de serializacion.

## Puerto
El puerto sera ADMIN_PORT

## Estructura de un mensaje
- **bytes**: 4 bytes que indican el tamano total del msg_t
- **type**: 1 Byte que representa la accion.
  - 0: Listar metricas
  - 1: Listar configuraciones
  - 2: Pedir metricas
  - 3: Pedir configuraciones
  - 4: Setear configuraciones
- **param**: 1 byte que indica (de ser necesario) de que metrica/configuracion se trata. El numero representado por el byte sera el numero de metrica/configuracion de sus respectivas listas.
- **buffer_size**: 4 bytes que indican (de existir un mensaje) el tamano del mensaje que sigue.
- **buffer**: mensaje del tamano de message_size.
### Los mensajes seran:
- La lista de metricas
- La lista de configuraciones
- El valor de una metrica
- El valor de una configuracion

## Breve descripcion de la implementacion
Se enviara a traves del socket una estructura msg_t con los campos mencionados en *Estructura de un mensaje*. Dependiendo del tipo de mensaje, puede que estos campos no esten inicializados y por lo tanto, no ocupen memoria. Los unicos campos que siempre estaran inicializados seran *bytes* y *type*. A estos se les agregan los demas dependiendo del tipo de mensaje:
- **LIST_METRCIS y LIST_CONFIGS**: Si se esta del lado del proxy: *buffer_size* y *buffer*.
- **GET_METRIC y GET_CONFIG**: *param* y si se esta del lado del proxy: *buffer_size* y *buffer*.
- **SET_CONFIG**: *param*
