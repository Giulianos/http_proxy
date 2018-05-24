add_library(Messages messages.c)
# Dependecies
 target_link_libraries(Messages Serializer)
