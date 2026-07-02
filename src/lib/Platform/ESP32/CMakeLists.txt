set(PROJ_ROOT ${CMAKE_SOURCE_DIR})

# Fallback: if CMAKE_SOURCE_DIR doesn't point to project root (ESP-IDF context),
# derive it from this file's location (src/lib/Platform/ESP32/CMakeLists.txt)
if(NOT EXISTS "${PROJ_ROOT}/src/lib/Platform/ESP32/CMakeLists.txt")
    get_filename_component(PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
endif()

include(${PROJ_ROOT}/fs.ll.cmake)

idf_component_register(
    SRCS
        "${PROJ_ROOT}/src/Sample.c"
        ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES driver esp_system
)
