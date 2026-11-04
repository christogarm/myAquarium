cmake_minimum_required(VERSION 3.16)

project(rtos_resources C)

add_library(rtos_resources STATIC
    rtos_resources.c
)

target_link_libraries(rtos_resources
    PUBLIC
        FreeRTOS-Kernel
)

target_include_directories(rtos_resources
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}
)