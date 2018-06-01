add_library(RequestParser requestParser.c)
# Dependecies
# target_link_libraries(RequestParser DependencyName)
target_link_libraries(RequestParser MyParserUtils)


