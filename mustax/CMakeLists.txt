cmake_minimum_required(VERSION 3.7)
project(mustax C)
set(CMAKE_C_STANDARD 11)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(${PROJECT_NAME} mustax.c)
