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

static const char *ResultMessage(FRESULT result) {
    switch (result) {
        case FR_OK:                  return "Succeeded";
        case FR_DISK_ERR:            return "A hard error occurred in the low level disk I/O layer";
        case FR_INT_ERR:             return "Assertion failed";
        case FR_NOT_READY:           return "The physical drive does not work";
        case FR_NO_FILE:             return "Could not find the file";
        case FR_NO_PATH:             return "Could not find the path";
        case FR_INVALID_NAME:        return "The path name format is invalid";
        case FR_DENIED:              return "Access denied due to a prohibited access or directory full";
        case FR_EXIST:               return "Access denied due to a prohibited access";
        case FR_INVALID_OBJECT:      return "The file/directory object is invalid";
        case FR_WRITE_PROTECTED:     return "The physical drive is write protected";
        case FR_INVALID_DRIVE:       return "The logical drive number is invalid";
        case FR_NOT_ENABLED:         return "The volume has no work area";
        case FR_NO_FILESYSTEM:       return "Could not find a valid FAT volume";
        case FR_MKFS_ABORTED:        return "The f_mkfs function aborted due to some problem";
        case FR_TIMEOUT:             return "Could not take control of the volume within defined period";
        case FR_LOCKED:              return "The operation is rejected according to the file sharing policy";
        case FR_NOT_ENOUGH_CORE:     return "LFN working buffer could not be allocated";
        case FR_TOO_MANY_OPEN_FILES: return "Number of open files exceeds the limit";
        case FR_INVALID_PARAMETER:   return "Given parameter is invalid";
        default:                     return "Unknown error";
    }
}

static FATFS fatfs;

bool MountSdCard(void) {
    if (!SDCardInit()) {
        printf("MountSdCard error: SDCardInit failed\n");
        return false;
    }

    FRESULT result = f_mount(&fatfs, SD_DRIVE, 1);
    if (result != FR_OK) {
        printf("MountSdCard error: %s\n", ResultMessage(result));
        return false;
    }
    return true;
}

bool SelectActiveDrive(void) {
    FRESULT result = f_chdrive(SD_DRIVE);
    if (result != FR_OK) {
        printf("SelectActiveDrive error: %s\n", ResultMessage(result));
        f_unmount(SD_DRIVE);
        return false;
    }
    return true;
}

bool OpenFile(FIL *file, const char *filename) {
    FRESULT result = f_open(file, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (result != FR_OK) {
        printf("OpenFile(%s) error: %s\n", filename, ResultMessage(result));
        f_unmount(SD_DRIVE);
        return false;
    }
    return true;
}

void CloseFile(FIL *file) {
    FRESULT result = f_close(file);
    if (result != FR_OK) {
        printf("CloseFile error: %s\n", ResultMessage(result));
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

bool PathOrFileExists(const char *path) {
    return f_stat(path, NULL) == FR_OK;
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
