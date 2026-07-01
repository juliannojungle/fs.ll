
# Platform-agnostic translation units, shared by both targets. Platform-specific
# sources (HAL.c, RTC.c, DiskIO.c) are added per-platform below.
set(FSLL_COMMON_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/lib/FileSystem.c
)

if(PLATFORM_NAME STREQUAL "ESP32")
    # ESP-IDF orchestrates the build.
    # EXTRA_COMPONENT_DIRS adds our component without losing ESP-IDF built-in components.
    set(EXTRA_COMPONENT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/ESP32"
    )
elseif(PLATFORM_NAME STREQUAL "RP2040")
    set(GUILL_RP2040_SRCS
        ${GUILL_COMMON_SRCS}
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/HAL.c
        ${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/RP2040/RTC.c
    )
endif()
