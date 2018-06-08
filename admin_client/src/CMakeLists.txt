# Subdirectories
add_subdirectory(actions)
add_subdirectory(serializer)
add_subdirectory(messages)

# Modules
include(FindSCTP.cmake)

find_package (Threads)

add_executable(admin_client main.c)

set_target_properties(admin_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../target)

# Libraries
target_link_libraries(admin_client ${SCTP_LIBRARY})
target_link_libraries(admin_client Actions)
target_link_libraries(admin_client Serializer)
target_link_libraries(admin_client Messages)
target_link_libraries (admin_client ${CMAKE_THREAD_LIBS_INIT})
