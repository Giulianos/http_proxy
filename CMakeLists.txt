cmake_minimum_required(VERSION 3.7)
project(proxy_http C)

# Turn verbose on
# set( CMAKE_VERBOSE_MAKEFILE on )

# Standard and compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -pedantic -pedantic-errors -Wall  -D_POSIX_C_SOURCE=200112L -O1")

# La version mas exigente
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread --std=c99 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -D_POSIX_C_SOURCE=200112L -O1 -fsanitize=address")


add_compile_options(-pedantic -Wall)

add_subdirectory(proxy_http/src)    