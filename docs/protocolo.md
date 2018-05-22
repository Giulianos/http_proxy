# Protocolo

Las tareas que necesitamos son:
- Listar metricas
- Listar configuraciones
- Pedir metricas
- Pedir configuraciones
- Setear configuraciones

## Resumen
Definimos entonces, un protocolo binario, orientado a conexion, de tipo *request/response*, donde en la request se envia la tarea que se quiere realizar y en la response el resultado de esta.

## Estructura de un mensaje
- **message_type**: 1 Byte que representa la accion.
  - 0: Listar metricas
  - 1: Listar configuraciones
  - 2: Pedir metricas
  - 3: Pedir configuraciones
  - 4: Setear configuraciones
- **param**: 1 byte que indica (de ser necesario) de que metrica/configuracion se trata. El numero representado por el byte sera el numero de metrica/configuracion de sus respectivas listas.
- **message_size**: 4 bytes que indican (de existir un mensaje) el tamano del mensaje que sigue.
- **message**: mensaje del tamano de message_size.
### Los mensajes seran:
- La lista de metricas
- La lista de configuraciones
- El valor de una metrica
- El valor de una configuracion
