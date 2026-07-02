# target configuration
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_rtc
    fatfs)
