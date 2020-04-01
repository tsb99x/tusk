cmake_minimum_required(VERSION 3.7)

project(tusk C)
set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(main src/main.c src/socket.c src/scgi.c src/utility.c)

if(WIN32)
    target_link_libraries(main ws2_32)
endif()

enable_testing()
add_executable(scgi_test src/scgi_test.c src/scgi.c)
add_test(scgi_test scgi_test)
