# version requirements
cmake_minimum_required(VERSION 3.16)

# Generate compile_commands.json (consumed by clangd for code intelligence)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-agnostic translation units, shared by both targets. Platform-specific
# sources (HAL.c, RTC.c, DiskIO.c) are added per-platform below.
set(FSLL_COMMON_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/Sample.c
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/FileSystem.c
)

# Platform selection
if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "RP2040")
endif()

# Optional debug messages: pass -DDEBUGMSGS (no value needed) at configure time to
# define the DEBUGMSGS macro for the build (used via #ifdef DEBUGMSGS in the sources).
# Guarded with DEFINED so it triggers whether CMake stores it as "ON" or as an empty value.
if(DEFINED DEBUGMSGS)
    add_compile_definitions(DEBUGMSGS)
endif()

# Build system per platform
if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build.
    # EXTRA_COMPONENT_DIRS adds our component without losing ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/ESP32"
    )
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(fs.ll)

elseif(PLATFORM_NAME STREQUAL "Simulator")
    set(CMAKE_C_STANDARD 11)
    project(fs.ll)

    set(FSLL_SIM_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/src/Sample.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/Simulator/HAL.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/Simulator/RTC.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/Simulator/DiskIO.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/FileSystem.c
        ${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ff.c
        ${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffsystem.c
        ${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffunicode.c
    )

    include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs.ffconf_patch.cmake)
    add_executable(${PROJECT_NAME} ${FSLL_SIM_SRCS})

    set(FSLL_LIB ${CMAKE_CURRENT_LIST_DIR}/src/lib)
    include_directories(${FSLL_LIB})
    include_directories(${FSLL_LIB}/Platform/Simulator)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source)

    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES} m)

else() # RP2040 (default)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 14)

    if(NOT DEFINED PICO_SDK_PATH)
        set(PICO_SDK_PATH ~/pico-sdk)
    endif()
    include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/pico_sdk_import.cmake)

    project(fs.ll)
    pico_sdk_init()

    # Include platform-specific cmake fragments
    include(${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/PreExecutable.cmake)

    add_executable(${PROJECT_NAME}
        ${FSLL_COMMON_SRCS}
        src/lib/Platform/RP2040/HAL.c
        src/lib/Platform/RP2040/RTC.c
    )

    include(${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/PostExecutable.cmake)

    pico_enable_stdio_usb(${PROJECT_NAME} 1)  # enable usb output
    pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
    pico_add_extra_outputs(${PROJECT_NAME})    # create map/bin/hex/uf2 files in addition to elf
endif()
