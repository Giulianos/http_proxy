# Subdirectories
add_subdirectory(selector)
add_subdirectory(buffer)
add_subdirectory(myParserUtils)
add_subdirectory(requestParser)
add_subdirectory(responseParser)

find_package (Threads)

add_executable(proxy_http main.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(proxy_http Selector)
target_link_libraries(proxy_http Buffer)
target_link_libraries(proxy_http MyParserUtils)
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http ResponseParser)
target_link_libraries (proxy_http ${CMAKE_THREAD_LIBS_INIT})
