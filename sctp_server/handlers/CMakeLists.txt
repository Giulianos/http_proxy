add_library(Handlers handlers.c)
# Dependecies
target_link_libraries(Handlers MSG_Queue)
target_link_libraries(Handlers Messages)
