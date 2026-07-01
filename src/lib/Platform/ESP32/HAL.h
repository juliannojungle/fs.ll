#ifndef HAL_H
#define HAL_H

/* Public interface of the ESP32 HAL. Definitions live in HAL.c. */
#include <stdint.h>
#include <stdbool.h>
#include "HALConfig.h"

void Delay(uint32_t milliseconds);
void STDIOInitAll(void);

#endif /* HAL_H */
