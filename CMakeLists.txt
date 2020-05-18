cmake_minimum_required(VERSION 3.10)
project(tusk LANGUAGES C)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

enable_testing()

add_subdirectory(rocker)
add_subdirectory(src)
