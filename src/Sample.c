#include "HAL.h"
#include "RTC.h"
#include "FileSystem.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

void app_entry(void) {
    STDIOInitAll();
    RTCInitialize();
    // Delay(3000); // give us time to start serial monitor

    FIL file;
    if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "st_benedict.txt")) {
        char buffer[1024];
        UINT bytesRead = ReadFile(&file, buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
        CloseFile(&file);
    }

    char* path = "tmp/01/02/03/test.txt";
    CreatePathDirectories(path);
    if (OpenFile(&file, path)) {
        char* text = "Crux Sacra Patris Benedicti";
        UINT bytesWritten = WriteFile(&file, text, strlen(text));
        CloseFile(&file);
    }

    if (OpenFile(&file, path)) {
        char buffer[1024];
        UINT bytesRead = ReadFile(&file, buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';
        printf("%s\n", buffer);
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
