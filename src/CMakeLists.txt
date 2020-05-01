set(templates
    ${CMAKE_CURRENT_LIST_DIR}/routes/hello.h
    ${CMAKE_CURRENT_LIST_DIR}/routes/login.h)
add_custom_command(
    OUTPUT ${templates}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../rocker/rocker ${templates}
    DEPENDS rocker)
add_custom_target(gen_templates
    DEPENDS ${templates})

add_executable(${PROJECT_NAME}
    main.c
        socket.c
        scgi.c
        utility.c
        routes/hello.c
        routes/login.c)
add_dependencies(${PROJECT_NAME}
    gen_templates)

if(WIN32)
    target_link_libraries(tusk ws2_32)
endif()

enable_testing()
add_executable(scgi_test
    scgi_test.c
        scgi.c)
add_test(scgi_test scgi_test)
