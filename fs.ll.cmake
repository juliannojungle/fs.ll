if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator")
endif()

if(NOT DEFINED PLATFORM_DIR)
    set(PLATFORM_DIR "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/${PLATFORM_NAME}")
endif()

set(SOURCES
    ${SOURCES}
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/FileSystem.c"
    "${PLATFORM_DIR}/HAL.c"
    "${PLATFORM_DIR}/RTC.c"
    "${PLATFORM_DIR}/DiskIO.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ff.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffsystem.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffunicode.c")

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/src/lib"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source")

# Apply patch to ffconf.h (FF_FS_RPATH=1, FF_VOLUMES=2, FF_CODE_PAGE=437, FF_USE_LFN=2)
include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs.ffconf_patch.cmake)
