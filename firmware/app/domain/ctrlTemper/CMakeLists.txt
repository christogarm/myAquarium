cmake_minimum_required(VERSION 3.16)

project(controlTemperature C)

add_library(ctrltemper STATIC
    ctrlTemper.c
)

target_link_libraries(ctrltemper
    PUBLIC
        pid
)

target_include_directories(ctrltemper
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)