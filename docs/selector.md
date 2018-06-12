# Uso de Selector

Como se explica en el archivo `proxy_http/src/selector/selector.h`, el selector permite **multiplexar** la entrada y salida utilizando `select`. Es decir, permite leer o escribir desde, o en, distintos file descriptors de manera **no bloqueante**. Para esto se debe proveer distintos handlers, los cuales son utilizados cuando hay algo para leer o escribir.

El selector tambien permite que se ejecuten tareas bloqueantes dentro de un handler (por ejemplo, la resolucion de un nombre por dns). Para esto dentro del handler se debe descargar esta tarea en un **nuevo thread** (con `pthread_create`)[1]. Dentro de este thread realizamos la tarea bloqueante y luego informamos al selector que la tarea ya finalizo, con `selector_notify_block`. Esto permite que el selector luego corra el *block_handler*, en el cual deberiamos utilizar el resultado de la operacion bloqueante.

**[1] Nota:** Es necesario a este nuevo thread desasociarlo con `pthread_detach`, esto permite que al terminar, se liberen sus recursos, sin tener que hacer pthread_join (similar al wait para los procesos).

Entonces el modo de uso del selector es el siguiente:
- Se hace `selector_init(...)` pasandole la estructura con la configuracion.
- Se **crea un nuevo selector** con `selector_new(...)`
- Luego podemos **registrar un file descriptor** en este selector con `selector_register(...)`, para esto tenemos que:
  - Configurar el fd como no bloqueante con `selector_fd_set_nio`.
  - Crear una estructura de tipo `fd_handler` con los handlers para cada evento (read, write, close y block)
  - Llamar a `selector_register` pasandole que selector vamos a usar, el fd que queremos registrar, la estructura creada, nuestro interes inicial (leer, escribir, ambos o ninguno) y un data que se le pasara a los handlers cada vez que sean llamados.
- Finalmente podemos hacer `selector_select` que se fija si en algun file descriptor registrado hay disponible algun interes configurado, si termino una tarea bloqueante o si se cerro el fd. En estos casos ejecuta el handler correspondiente.

A selector_select se lo debe llamar siempre para que realize una iteracion por todos los file descriptors.

Ademas, se pueden realizar otras operaciones sobre un selector, como por ejemplo, cambiar los intereses sobre un fd o desregistrarlo. Ver `proxy_http/src/selector/selector.h` para mas detalles.
