# TP de Protocolos de Comunicacion
En este manual se describen los pasos para compilar y ejecutar las dos aplicaciones que conforman el TPE.

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


#### Cliente
La compilacion del cliente se realiza de la mismo forma que para el servidor. De forma manual se puede compilar con los siguientes pasos:
```
$ cd admin_client
$ mkdir build
$ cd build
$ cmake ../
$ make
```
Se incluye tambien un script bash dentro de `admin_client` que realiza todos estos pasos de forma automatica. Por lo que se puede ejecutar:
```
$ cd admin_client
$ ./compile.sh
```
En ambos casos el archivo ejecutable queda dentro de `admin_client/build/target`

## Ejecución

### Servidor
El servidor recibe los parámetros detallados en el archivo httpd.8 provisto por la cátedra. (ver con man ./httpd.8).

Ejemplo de ejecucion:
```
$ cd proxy_http/build/target
$ ./proxy_http -h
```
Lo cual nos da como salida los parametros aceptados.

### Cliente
El cliente permite configurar la ip y el puerto del servidor al que se conectará. El modo de ejecución es el siguiente:
```
$ cd admin_client/build/target
$ ./admin_client 127.0.0.1 9090
```
