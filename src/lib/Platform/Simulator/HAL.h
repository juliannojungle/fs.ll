#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include "Types.h"
#include "HALConfig.h"

void Delay(UINT32 milliseconds);
void STDIOInitAll(void);

#endif /* HAL_H */
