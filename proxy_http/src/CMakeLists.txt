# Subdirectories
add_subdirectory(selector )
add_subdirectory(logger)

find_package (Threads)

add_executable(proxy_http main.c argument/argument.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(proxy_http Selector Logger)


target_link_libraries (proxy_http ${CMAKE_THREAD_LIBS_INIT})
