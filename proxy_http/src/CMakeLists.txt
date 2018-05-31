# Subdirectories
add_subdirectory(response_parser)
add_subdirectory(request_parser)
add_subdirectory(buffer)
add_subdirectory(client)
add_subdirectory(selector)
add_subdirectory(myParserUtils)

find_package (Threads)

add_executable(proxy_http main.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(proxy_http ResponseParser)
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http Buffer)
target_link_libraries(proxy_http Client)
target_link_libraries(proxy_http Selector)
target_link_libraries(proxy_http MyParserUtils)
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http ResponseParser)
target_link_libraries (proxy_http ${CMAKE_THREAD_LIBS_INIT})

# Tests
add_executable(tests tests/requestParserTest.c)
set_target_properties(tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
target_link_libraries(tests Buffer)
target_link_libraries(tests MyParserUtils)
