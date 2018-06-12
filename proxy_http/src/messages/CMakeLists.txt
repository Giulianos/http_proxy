add_library(Messages messages.c)

# Modules
include(FindSCTP.cmake)

# Dependecies
target_link_libraries(Messages Serializer)
target_link_libraries(Messages ${SCTP_LIBRARIES})