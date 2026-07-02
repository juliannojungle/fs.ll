cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generate compile_commands.json (consumed by clangd for code intelligence)

set(PROJECT_NAME "fs.ll")
if(NOT PLATFORM_NAME STREQUAL "ESP32")
    project(${PROJECT_NAME})
endif()
include(${CMAKE_SOURCE_DIR}/fs.ll.cmake)

set(SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/Sample.c"
    ${SOURCES})

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build: EXTRA_COMPONENT_DIRS adds our component keeping ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/src/lib/Platform/ESP32")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(${PROJECT_NAME})

elseif(PLATFORM_NAME STREQUAL "Simulator")
    set(CMAKE_C_STANDARD 11)
    add_executable(${PROJECT_NAME} ${SOURCES})
    include_directories(${INCLUDE_DIRS})

elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH ~/pico-sdk)
    endif()
    include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/pico_sdk_import.cmake)

    pico_sdk_init()
    add_executable(${PROJECT_NAME} ${SOURCES})
    include_directories(${INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} PUBLIC
        pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_rtc)

    pico_enable_stdio_usb(${PROJECT_NAME} 1)  # enable usb output
    pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
    pico_add_extra_outputs(${PROJECT_NAME})   # create map/bin/hex/uf2 files in addition to elf
endif()
