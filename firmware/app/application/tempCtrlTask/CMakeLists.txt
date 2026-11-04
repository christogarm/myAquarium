cmake_minimum_required(VERSION 3.16)

project(controlTemperatureTask C)

add_library(tempCtrlTask STATIC
    tempCtrlTask.c
)

target_link_libraries(tempCtrlTask
    PUBLIC
        ctrltemper
        FreeRTOS-Kernel
        pico_stdlib
        config
        rtos_resources
)

target_include_directories(tempCtrlTask
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}
)