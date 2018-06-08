cmake_minimum_required(VERSION 3.7)
project(sctp_server C)

set(CMAKE_C_STANDARD 99)

include(FindSCTP.cmake)
include_directories(.)

add_subdirectory(messages)
add_subdirectory(serializer)
add_subdirectory(config)
add_subdirectory(metric)
add_subdirectory(selector)
add_subdirectory(handlers)
add_subdirectory(actions)
add_subdirectory(msg_queue)

find_package(Threads)
add_executable(sctp_server main.c)


target_link_libraries(sctp_server ${SCTP_LIBRARIES})
target_link_libraries(sctp_server Serializer)
target_link_libraries(sctp_server Messages)
target_link_libraries(sctp_server Config)
target_link_libraries(sctp_server Metric)
target_link_libraries(sctp_server Selector)
target_link_libraries(sctp_server Handlers)
target_link_libraries(sctp_server Actions)
target_link_libraries(sctp_server MSG_Queue)
target_link_libraries(sctp_server ${CMAKE_THREAD_LIBS_INIT})
