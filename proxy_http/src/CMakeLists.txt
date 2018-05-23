# Subdirectories
add_subdirectory(request_parser)
add_subdirectory(buffer)
add_subdirectory(client)
add_subdirectory(selector)

find_package (Threads)

add_executable(proxy_http main.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http Buffer)
target_link_libraries(proxy_http Client)
target_link_libraries(proxy_http Selector)
target_link_libraries (proxy_http ${CMAKE_THREAD_LIBS_INIT})
