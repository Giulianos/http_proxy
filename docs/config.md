# Modulo Config

Este modulo se encarga de almacenar configuraciones.

- **struct config**: Tiene dos campos: *name* es el id de la configuracion; *value* el valor que esta configuracion tiene seteado. El valor es de tipo char * dado que puede ser de este tipo o int, en cuyo caso se utiliza la funcion atoi.
- **configurations**: Tiene todas las configuraciones que fueron seteadas.
- **MAX_CONFIG**: Se soporta hasta un maximo de MAX_CONFIG por lo que se cuenta con un array *configurations* de este tamano.
- **MAX_NAME**: Se soporta hasta un maximo de caracteres en el configuration name MAX_NAME. En caso de que pasen un name con mayor tamano, si coinciden en MAX_NAME, tomara que se trata de este mismo.
- **NAME**: Acepta letras, numeros, '_', '-'

## Config Initialization

Levanta de un archivo configuraciones seteadas default para que el proxy funcione correctamente. El formato del archivo debe ser:
```
name1=value1
name2=value2
...
```
