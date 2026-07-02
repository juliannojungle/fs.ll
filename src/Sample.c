#include "HAL.h"
#include "RTC.h"
#include "FileSystem.h"
#include <stdio.h>

void app_entry(void) {
    STDIOInitAll();
    RTCInitialize();
    // Delay(3000); // give us time to start serial monitor

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "st_benedict.txt")) {
        char buffer[1024];
        UINT bytesRead;
        f_read(&file, buffer, sizeof(buffer) - 1, &bytesRead);
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
        CloseFile(&file);
    }
    UnMountSdCard();
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
