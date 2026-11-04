cmake_minimum_required(VERSION 3.16)

project(temperature C)

add_library(temperature STATIC
    temperature.c
)

target_link_libraries(temperature
    PUBLIC
        FreeRTOS-Kernel
        pico_stdlib
)

target_include_directories(temperature
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}
)