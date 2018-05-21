# Modulo Client

El modulo client se encarga de atender a los clientes. Internamente funciona como una maquina de estados. Requiere que se le informe de los eventos del selector. Estos eventos ejecutan distintas acciones dependiendo del estado actual del cliente. Para esto, el modulo expone cuatro handlers para cada uno de los posibles eventos sobre el file descriptor del socket cliente. Internamente tambien se usa otro file descriptor para el socket utilizado en la conexion con el host remoto (el que esta solicitando el cliente). La maquina de estados tambien evoluciona frente a eventos en este file descriptor, pero en el contrato no se exponen los handlers pues el registro del file descriptor en el selector lo hace el modulo internamente.

##  Estados
Los estados que maneja internamente el modulo client en su maquina de estados son:

- **NO_HOST:** Cuando aun no se parseo el host de la request.
- **HOST_RESOLV:** Cuando se esta resolviendo el host por DNS.
- **READ_REQ:** Cuando se esta leyendo la request desde el cliente.
- **READ_RESP:** Cuando se esta leyendo la respuesta desde el servidor remoto.
- **NO_REMOTE:** Cuando el host remoto cerro la conexion.
- **ERROR:** Cuando se produjo un error.

Ademas se cuenta con dos flags que indican si ya se leyo completamente la request/response actual, estos son: `request_complete` y `response_complete`. Estos flags son seteados por el parser.

## Acciones ante eventos
Los siguientes son los posibles eventos que pueden llegar desde el selector y las acciones que toma la maquina de estados dependiendo del estado actual.

Aclaracion: cuando se pasa la request por request parser, este deja los caracteres consumidos en el buffer de entrada. Cuando se pasa la response por el response parser, este deja los caracteres consumidos en el buffer de salida.

#### Leer del cliente
- **NO_HOST:** Pasamos la request por el request parser. Una vez obtenido el host, pasamos al estado **HOST_RESOLV** y descargamos el trabajo de resolucion de nombres en otro thread.
- **READ_REQ:** Pasamos la request por el request parser.
- **NO_REMOTE:** Respondemos con el error correspondiente.

En el resto de los estados no hacemos nada ante este evento.

#### Escribir hacia el cliente
- **READ_RESP:** Si hay caracteres en el buffer de salida, los enviamos al cliente. Si no hay caracteres en el buffer de salida y esta seteado `response_complete`  pasamos al estado **NO_HOST** (restaurando el valor de los flags).
- **NO_REMOTE:** Si hay caracteres en el buffer de salida, los enviamos al cliente. Si no hay caracteres en el buffer de salida liberamos los recursos (igual que en close del cliente) y hacemos close del cliente.
- **ERROR:** Chequeamos que error se produjo, y si es necesario, se envia al cliente una response acorde.

En el resto de los estados no hacemos nada ante este evento.

#### Desbloqueo en el cliente
- **HOST_RESOLV:** Hacemos connect al servidor remoto y pasamos al estado **READ_REQ**.

En el resto de los estados no hacemos nada ante este evento.

#### Close en el cliente
Liberamos los recursos del cliente (buffers, parser, socket al remoto, fd's, etc.).

#### Leer en remoto
- **READ_RESP:** Pasamos la response por el response parser.

#### Escribir en remoto
- **READ_REQ:** Si hay caracteres en el buffer de entrada, los enviamos al remoto. Si no hay caracteres en el buffer de entrada y esta seteado `request_complete` pasamos al estado **READ_RESP**.

#### Close en remoto
- **READ_RESP:** Desregistramos al remoto del selector y vamos al estado **NO_REMOTE**.

## Implementacion
Aca se detallan notas de la implementacion

#### TO DO:
 - Falta implementar los handlers para los eventos en el fd del host remoto.
 - Falta chequear errores
 - Falta revisar como responder errores (tal vez se necesita agregar un estado "**SENDING_ERROR**")
 - Falta acoplar los parsers
 - Crear los buffers sacando los tamaños del modulo Config.
 - Restaurar el estado del cliente (`client_restart_state`)
 - Liberar recursos (`client_free_resources`)
 - Crear thread para resolver host
 - Intentar conectar con todas las ip's obtenidas


#### Dudas:
- Que pasa si al enviar la request al servidor remoto, este cierra la conexion sin enviar ninguna response?
  - Cerramos  la conexion al cliente sin enviar respuesta
  - Enviamos un codigo de error (5xx??)
