# Subdirectories
add_subdirectory(transformations)
add_subdirectory(response_parser)
add_subdirectory(request_parser)
add_subdirectory(buffer)
add_subdirectory(client)
add_subdirectory(selector)
add_subdirectory(myParserUtils)
add_subdirectory(logger)
add_subdirectory(messages)
add_subdirectory(serializer)
add_subdirectory(config)
add_subdirectory(metric)
add_subdirectory(admin_handlers)
add_subdirectory(admin_actions)
add_subdirectory(msg_queue)
add_subdirectory(argument)

find_package (Threads)
include(FindSCTP.cmake)

add_executable(proxy_http main.c)

set_target_properties(proxy_http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries

target_link_libraries(proxy_http ${SCTP_LIBRARIES})
target_link_libraries(proxy_http Serializer)
target_link_libraries(proxy_http Messages)
target_link_libraries(proxy_http Metric)
target_link_libraries(proxy_http Handlers)
target_link_libraries(proxy_http Actions)
target_link_libraries(proxy_http MSG_Queue)
target_link_libraries(proxy_http Transformations)
target_link_libraries(proxy_http Config)
target_link_libraries(proxy_http ResponseParser)
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http Buffer)
target_link_libraries(proxy_http Client)
target_link_libraries(proxy_http Selector)
target_link_libraries(proxy_http MyParserUtils)
target_link_libraries(proxy_http RequestParser)
target_link_libraries(proxy_http ResponseParser)
target_link_libraries(proxy_http Logger)
target_link_libraries(proxy_http Argument)

target_link_libraries(proxy_http ${CMAKE_THREAD_LIBS_INIT})

# Tests
add_executable(bufferTest tests/bufferTest.c)
add_executable(myParserUtilsTest tests/myParserUtilsTest.c)
add_executable(requestParserTest tests/requestParserTest.c)
add_executable(responseParserTest tests/responseParserTest.c)
add_executable(transformations_test tests/transformation_test.c)

set_target_properties(bufferTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(myParserUtilsTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(requestParserTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(responseParserTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)
set_target_properties(transformations_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Tests Libraries
target_link_libraries(bufferTest Buffer)

target_link_libraries(myParserUtilsTest Buffer)
target_link_libraries(myParserUtilsTest MyParserUtils)

target_link_libraries(requestParserTest Buffer)
target_link_libraries(requestParserTest MyParserUtils)
target_link_libraries(requestParserTest RequestParser)

target_link_libraries(responseParserTest Buffer)
target_link_libraries(responseParserTest MyParserUtils)
target_link_libraries(responseParserTest ResponseParser)

target_link_libraries(transformations_test Transformations)
