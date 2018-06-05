# Subdirectories
add_subdirectory(response_parser)
add_subdirectory(request_parser)
add_subdirectory(buffer)
add_subdirectory(client)
add_subdirectory(selector)
add_subdirectory(myParserUtils)
add_subdirectory(logger)

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
target_link_libraries(proxy_http Logger)
target_link_libraries(proxy_http ${CMAKE_THREAD_LIBS_INIT})

# Tests
add_executable(bufferTest tests/bufferTest.c)
add_executable(myParserUtilsTest tests/myParserUtilsTest.c)
add_executable(requestParserTest tests/requestParserTest.c)
#add_executable(responseParserTest tests/responseParserTest.c)

set_target_properties(bufferTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(myParserUtilsTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(requestParserTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
#set_target_properties(responseParserTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Tests Libraries
target_link_libraries(bufferTest Buffer)

target_link_libraries(myParserUtilsTest Buffer)
target_link_libraries(myParserUtilsTest MyParserUtils)

target_link_libraries(requestParserTest Buffer)
target_link_libraries(requestParserTest MyParserUtils)
target_link_libraries(requestParserTest RequestParser)

#target_link_libraries(responseParserTest Buffer)
#target_link_libraries(responseParserTest MyParserUtils)
#target_link_libraries(responseParserTest ResponseParser)
