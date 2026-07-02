#ifndef HAL_H
#define HAL_H

/* Public interface of the RP2040 HAL. Definitions live in HAL.c. */
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "HALConfig.h"

void Delay(unsigned int milliseconds);
void STDIOInitAll(void);

#endif /* HAL_H */
