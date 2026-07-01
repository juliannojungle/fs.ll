#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdbool.h>

#define SD_SPI_CS       6
#define SD_SPI_SCLK     7
#define SD_SPI_MOSI     8
#define SD_SPI_MISO     9
#define SD_SPI_BAUDRATE 25000000
#define SD_DETECT_PIN   10

#define SD_DISK_IMAGE "sample/sdcard.img"

#endif /* HAL_CONFIG_H */
