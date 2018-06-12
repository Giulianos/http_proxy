add_library(Client client.c remote_handlers.c remote_handlers.h client_handlers.c transformation_handlers.c transformation_handlers.h chunks.c)
# Dependecies
target_link_libraries(Client Buffer)
target_link_libraries(Client RequestParser)
target_link_libraries(Client ResponseParser)
target_link_libraries(Client Transformations)