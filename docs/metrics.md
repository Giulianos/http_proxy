## Métricas
Las metricas se representan por un numero de la siguiente manera:
```
enum metrics{
  INST_CONCURRENT_CONNECTIONS,
  MAX_CONCURRENT_CONNECTIONS,
  ACCESSES,
  TRANSFERED_BYTES,
  AVG_CONNECTION_TIME
}
```
Se cuenta tambien con la funcion ```metric_get_name``` que recibe un numero de metrica y devuelve el nombre que la representa. Esta se usa cuando se envia el mensaje al cliente administrador.
El valor de las metricas es de tipo *double*, y se cuenta con una funcion ```metric_get_value_string```que devuelve un string con el value. Esta se usa cuando se envia el mensaje al cliente administrador.
El codigo de error para cuando una metrica no es encontrada es el -1, ya que las metricas son siempre cantidades o tiempos y no pueden ser negativas.

- **Conexiones concurrentes:**
Lleva registro la cantidad de clientes que hay conectados (en ese instante) al proxy. Para esto, cada vez que un nuevo cliente se conecta, se llama a la funcion ```new_connection``` la cual incrementa en uno el valor de la metrica *INST_CONCURRENT_CONNECTIONS*. A su vez, cuando un cliente finaliza su conexion, se llama a la funcion ```end_connection``` que decrementa en uno el mismo valor.

- **Maximo de conexiones concurrentes:**
Lleva registro de la maxima cantidad de conexiones concurrentes que hubo hasta el momento. Para esto, cada vez que un nuevo cliente se conecta, en la funcion ```new_connection``` mencionada anteriormente, se compara el valor resultante de *INST_CONCURRENT_CONNECTIONS* con el valor de *MAX_CONCURRENT_CONNECTIONS*. Si el primero supera al segundo, entonces se actualiza el nuevo maximo.

- **Cantidad de accesos historicos:**
Lleva registro de la cantidad de clientes que se tuvo sin contar los que aun estan siendo atendidos. Para esto, en cada llamada a la funcion ```end_connection``` aumenta en uno el valor de la metrica *ACCESSES*.

- **Bytes transferidos:**
Lleva registro de la cantidad de bytes que fueron transferidos por el proxy, tanto hacia el client como hacia el origin. Para esto, luego de la transferencia, se llama a la funcion ```add_transfered_bytes``` la cual suma al valor *TRANSFERED_BYTES* la cantidad de bytes transferidos.

- **Avg connection time:**
Lleva registro del promedio de tiempo en que un cliente se mantiene conectado al proxy. Para esto, en cada llamada a ```new_connection``` se crea una estructura de tipo *connection_metrics_t* y se almacena el tiempo inicial de ese cliente. Luego, en cada llamada a ```end_connection``` se pasa por parametro esta estructura y se calcula con el tiempo actual y el tiempo almacenado en la estructura, cuanto tiempo estuvo ese cliente conectado. Luego, utilizando la variable estatica *connection_time_sum* (que lleva registro de la suma de los tiempos de atencion a cada cliente), se almacena en *AVG_CONNECTION_TIME* el valor de *connection_time* / *ACCESSES* (luego de sumarle a *ACCESSES* el cliente actual).
