cmake_minimum_required(VERSION 3.7)
project(tusk C)
set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(mustax
    src/mustax.c)

set(templates
    ${CMAKE_CURRENT_SOURCE_DIR}/src/routes/hello.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/routes/login.h)
add_custom_command(
    OUTPUT ${templates}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mustax ${templates}
    DEPENDS mustax)
add_custom_target(gen_templates
    DEPENDS ${templates})

add_executable(tusk
    src/main.c
        src/socket.c
        src/scgi.c
        src/utility.c
        src/routes/hello.c
        src/routes/login.c)
add_dependencies(tusk gen_templates)

if(WIN32)
    target_link_libraries(tusk ws2_32)
endif()

enable_testing()
add_executable(scgi_test src/scgi_test.c src/scgi.c)
add_test(scgi_test scgi_test)
