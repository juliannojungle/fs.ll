#ifndef _HAL_C_
#define _HAL_C_

#include "HAL.h"
#include "pico/stdio_usb.h"

void Delay(unsigned int milliseconds) {
    sleep_ms(milliseconds);
}

void STDIOInitAll(void) {
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
}

#endif /* _HAL_C_ */