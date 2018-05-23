# Protocolo

Las tareas que necesitamos son:
- Listar metricas
- Listar configuraciones
- Pedir metricas
- Pedir configuraciones
- Setear configuraciones

## Resumen
Definimos entonces, un protocolo binario, no orientado a conexion, de tipo *request/response*, donde en la request se envia la tarea que se quiere realizar y en la response el resultado de esta.

## Estructura de un mensaje
- **message_type**: 1 Byte que representa la accion.
  - 0: Desconocido (cuando se desconoce la request)
  - 1: Autenticarse
  - 1: Listar metricas
  - 2: Listar configuraciones
  - 3: Pedir metricas
  - 4: Pedir configuraciones
  - 5: Setear configuraciones
  - 6: No autenticado
- **cookie**: 1 byte que indica identificador que entrega el servidor. Si es 0 es porque no esta autenticado.
- **message_size**: 4 bytes que indican (de existir un mensaje) el tamano del mensaje que sigue.
- **message**: mensaje del tamano de message_size.
### Los mensajes seran:
- La lista de metricas
- La lista de configuraciones
- El valor de una metrica
- El valor de una configuracion


https://pastebin.com/raw/3k7uqUAJ

