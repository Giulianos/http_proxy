add_library(Actions admin_actions.c)
# Dependecies
target_link_libraries(Actions Config)
target_link_libraries(Actions Metric)
