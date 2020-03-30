cmake_minimum_required(VERSION 3.7)

project(tusk C)
set(CMAKE_C_STANDARD 11)
add_executable(main src/main.c src/socket.c src/utility.c)

if (WIN32)
    target_link_libraries(main ws2_32)
endif (WIN32)

enable_testing()
add_executable(test_main src/test_main.c)
add_test(test_main test_main)
