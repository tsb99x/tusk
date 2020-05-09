set(templates
    ${CMAKE_CURRENT_LIST_DIR}/hello.h
    ${CMAKE_CURRENT_LIST_DIR}/login.h)
add_custom_command(
    OUTPUT ${templates}
    COMMAND $<TARGET_FILE:rocker> ${templates}
    DEPENDS rocker)
add_custom_target(gen_templates
    DEPENDS ${templates})

add_library(routes hello.c login.c)
add_dependencies(routes gen_templates)
target_link_libraries(routes scgi)
target_include_directories(routes PUBLIC include)
