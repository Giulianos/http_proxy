add_library(Actions actions.c)
# Dependecies
target_link_libraries(Actions Config)
target_link_libraries(Actions Metric)
