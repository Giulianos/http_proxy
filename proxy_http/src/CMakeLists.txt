# Subdirectories
add_subdirectory(config)
add_subdirectory(selector)

find_package (Threads)

add_executable(proxy_http main.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(proxy_http Config)
target_link_libraries(proxy_http Selector)
target_link_libraries (proxy_http ${CMAKE_THREAD_LIBS_INIT})
