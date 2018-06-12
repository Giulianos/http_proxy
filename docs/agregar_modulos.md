# Como agregar modulos al tp

## Con script bash
Si se quiere crear el modulo MiModulo con sus archivos fuente dentro de mi_modulo, correr el script:
```bash
$ ./new_module.sh MiModulo mi_modulo
```
El script crea los directorios, el CMakeLists.txt, y agrega las dependecias necesarias en el CMakeLists.txt del directorio src. Ademas provee tanto un contrato dentro de `src/include/mi_modulo` de ejemplo como tambien un `source_sample.c` de ejemplo dentro de la carpeta del modulo.

## Forma manual
Para agregar un modulo se deben seguir los siguientes pasos:

- Crear una carpeta con el nombre del modulo (mi_modulo) dentro de src (aca van a ir los .c)
- Crear un `CMakeLists.txt` dentro de esta carpeta con el siguiente contenido:
  ```cmake
  add_library(MiModulo source1.c source2.c ...)
  ```
- Agregar en la seccion Subdirectores del CMakeLists.txt dentro de src el siguiente contenido:
  ```cmake
  add_subdirectory(mi_modulo source1.c source2.c ...)
  ```
- Agregar en la seccion Libraries del CMakeLists.txt dentro de src el siguiente contenido:
  ```cmake
  target_link_libraries(proxy_http MiModulo)
  ```

Para agregar dependecias al modulo, agregar al CMakeLists.txt del modulo lo siguiente:
```cmake
target_link_libraries(MiModulo OtroModulo)
```

Si el modulo debe proveer un contrato, este deberia ir dentro de `include/mi_modulo`. Luego se puede incluir desde otros modulos con `#include <mi_modulo/mi_modulo.h>`.
