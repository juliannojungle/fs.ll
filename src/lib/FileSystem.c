#ifndef __FILESYSTEM_
#define __FILESYSTEM_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ff.h"
#include "FileSystem.h"

/* SDCardInit is defined in the platform DiskIO.c, which is compiled as a
 * separate translation unit (linked via the fatfs library) rather than being
 * #included into this one. Forward-declared here (in the caller) so we keep
 * function declarations out of headers — see AGENTS.md §5. */
bool SDCardInit(void);

/* FatFS logical volume name (physical drive 0). This is a FatFS-level concept,
 * identical across platforms, so it lives here rather than in HALConfig.h. */
#define SD_DRIVE "0:"

/* -----------------------------------------------------------------------
 * File system helper functions (single SD card on FatFS volume SD_DRIVE)
 * ----------------------------------------------------------------------- */

static FATFS fatfs;

bool MountSdCard(void) {
    if (!SDCardInit()) {
        printf("SDCardInit failed\n");
        return false;
    }

    FRESULT result = f_mount(&fatfs, SD_DRIVE, 1);
    if (result != FR_OK) {
        printf("f_mount error: %d\n", result);
        return false;
    }
    return true;
}

bool SelectActiveDrive(void) {
    FRESULT result = f_chdrive(SD_DRIVE);
    if (result != FR_OK) {
        printf("f_chdrive error: %d\n", result);
        f_unmount(SD_DRIVE);
        return false;
    }
    return true;
}

bool OpenFile(FIL *file, const char *filename) {
    FRESULT result = f_open(file, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (result != FR_OK && result != FR_EXIST) {
        printf("f_open(%s) error: %d\n", filename, result);
        f_unmount(SD_DRIVE);
        return false;
    }
    return true;
}

void CloseFile(FIL *file) {
    FRESULT result = f_close(file);
    if (result != FR_OK) {
        printf("f_close error: %d\n", result);
    }
}

unsigned int ReadFile(FIL *file, void *buffer, unsigned int bytesToRead) {
    UINT bytesRead;
    f_read(file, buffer, bytesToRead, &bytesRead);
    return bytesRead;
}

unsigned int WriteFile(FIL *file, void *buffer, unsigned int bytesToWrite) {
    UINT bytesWritten;
    f_write(file, buffer, bytesToWrite, &bytesWritten);
    return bytesWritten;
}

void CreatePathDirectories(const char *path) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            f_mkdir(tmp);
            *p = '/';
        }
    }
}

void UnMountSdCard(void) {
    f_unmount(SD_DRIVE);
}

#endif /* __FILESYSTEM_ */
