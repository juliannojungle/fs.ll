# target configuration
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)

###### gui.ll
set(FSLL_LIB ${CMAKE_CURRENT_LIST_DIR}/../../../lib)
include_directories(${FSLL_LIB})
include_directories(${FSLL_LIB}/Platform/RP2040)
###### END gui.ll

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_rtc
    fatfs)
