cmake_minimum_required(VERSION 3.7)
project(proxy_http C)

# Turn verbose on
# set( CMAKE_VERBOSE_MAKEFILE on )

# Standard and compiler flags
set(CMAKE_C_STANDARD 99)
add_compile_options(-pedantic -Wall -O1)


include_directories(src/include)

add_subdirectory(src)
