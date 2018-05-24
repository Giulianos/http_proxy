# Subdirectories
add_subdirectory(handlers)
add_subdirectory(print_queue)
add_subdirectory(actions)
add_subdirectory(msg_queue)
add_subdirectory(serializer)
add_subdirectory(messages)
add_subdirectory(selector)

find_package (Threads)

add_executable(admin_client main.c)

set_target_properties(admin_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(admin_client Handlers)
target_link_libraries(admin_client PrintQueue)
target_link_libraries(admin_client Actions)
target_link_libraries(admin_client MSG_Queue)
target_link_libraries(admin_client Serializer)
target_link_libraries(admin_client Messages)
target_link_libraries(admin_client Selector)
target_link_libraries (admin_client ${CMAKE_THREAD_LIBS_INIT})
