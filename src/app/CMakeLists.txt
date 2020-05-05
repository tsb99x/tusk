add_executable(${PROJECT_NAME} main.c)
add_dependencies(${PROJECT_NAME} gen_templates)
target_link_libraries(${PROJECT_NAME} utility routes)

enable_testing()
