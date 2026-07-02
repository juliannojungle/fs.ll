#include "HAL.h"
#include <unistd.h>

void Delay(unsigned int milliseconds) {
    sleep(milliseconds / 1000);
}
void STDIOInitAll(void) {}
