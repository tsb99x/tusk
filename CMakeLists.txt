cmake_minimum_required(VERSION 3.7)
project(tusk C)
set(CMAKE_C_STANDARD 11)

add_subdirectory(rocker)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(templates
    ${CMAKE_CURRENT_SOURCE_DIR}/src/routes/hello.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/routes/login.h)
add_custom_command(
    OUTPUT ${templates}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rocker/rocker ${templates}
    DEPENDS rocker)
add_custom_target(gen_templates
    DEPENDS ${templates})

add_executable(${PROJECT_NAME}
    src/main.c
        src/socket.c
        src/scgi.c
        src/utility.c
        src/routes/hello.c
        src/routes/login.c)
add_dependencies(${PROJECT_NAME}
    gen_templates)

if(WIN32)
    target_link_libraries(tusk ws2_32)
endif()

enable_testing()
add_executable(scgi_test
    src/scgi_test.c 
        src/scgi.c)
add_test(scgi_test scgi_test)
