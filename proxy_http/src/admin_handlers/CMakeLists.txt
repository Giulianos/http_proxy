add_library(Handlers admin_handlers.c)
# Dependecies
target_link_libraries(Handlers MSG_Queue)
target_link_libraries(Handlers Messages)
