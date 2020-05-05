add_library(scgi scgi.c)
target_link_libraries(scgi socket)
target_include_directories(scgi PUBLIC include)

add_executable(scgi_test scgi_test.c)
target_link_libraries(scgi_test scgi)
add_test(scgi_test scgi_test)
