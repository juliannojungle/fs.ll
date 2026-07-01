#include "HAL.h"
#include "RTC.h"
#include "FileSystem.h"
#include <stdlib.h>
#include <stdio.h>

void app_entry(void) {
    STDIOInitAll();
    RTCInitialize();
    // Delay(3000); // give us time to start serial monitor

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "st_benedict.txt")) {
        // print file content here
        CloseFile(&file);
    }
    UnMountSdCard();

    while(true) {
    }
}

#ifdef ESP_PLATFORM
void app_main(void) {
    app_entry();
}
#else
int main(void) {
    app_entry();
    return 0;
}
#endif
