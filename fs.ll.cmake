if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator")
endif()

set(SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/FileSystem.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/${PLATFORM_NAME}/HAL.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/${PLATFORM_NAME}/RTC.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/${PLATFORM_NAME}/DiskIO.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ff.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffsystem.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source/ffunicode.c"
)

set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/src/lib"
    "${CMAKE_CURRENT_LIST_DIR}/src/lib/Platform/${PLATFORM_NAME}"
    "${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs/source"
)

# Apply patch to ffconf.h (FF_FS_RPATH=1, FF_VOLUMES=2)
include(${CMAKE_CURRENT_LIST_DIR}/src/Dependency/fatfs.ffconf_patch.cmake)
