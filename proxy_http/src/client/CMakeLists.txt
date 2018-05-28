add_library(Client client.c remote_handlers.c remote_handlers.h client_handlers.c)
# Dependecies
target_link_libraries(Client Buffer)
target_link_libraries(Client RequestParser)
target_link_libraries(Client ResponseParser)