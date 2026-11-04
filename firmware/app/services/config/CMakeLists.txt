cmake_minimum_required(VERSION 3.16)

project(config C)

add_library(config STATIC
    config.c
)

target_link_libraries(config
    PUBLIC
        pid
)

target_include_directories(config
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}
)