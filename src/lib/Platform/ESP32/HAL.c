#ifndef _HAL_C_
#define _HAL_C_

#include "HAL.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void Delay(uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

void STDIOInitAll(void) {
    // ESP-IDF initializes stdio automatically via the console component.
}

#endif /* _HAL_C_ */
