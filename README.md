# TP de Protocolos de Comunicacion

## Compilacion
La compilacion se realiza utilizando cmake. Se requiere al menos la version 3.7.
#### Servidor
De forma manual el servidor se puede compilar con los siguientes pasos:
```
$ cd proxy_http
$ mkdir build
$ cd build
$ cmake ../
$ make
```
Se incluye tambien un script bash dentro de `proxy_http` que realiza todos estos pasos de forma automatica. Por lo que se puede ejecutar:
```
$ cd proxy_http
$ ./compile.sh
```
En ambos casos el archivo ejecutable queda dentro de `proxy_http/build/target`

## Servidor Proxy HTTP
El servidor proxy actualmente recibe conexiones en el puerto TCP 8080. A modo de prueba, al recibir una conexion se la acepta, se muestra un mensaje en la consola del servidor y se cierra el socket.

Para poder hacer esto, el servidor utiliza un selector (implementacion provista por Juan F. Codagnone), el cual permite multiplexar la entrada y salida. Para mas informacion sobre el uso del selector ver archivo `docs/selector.md`.
