#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_

/* Public interface of the SD/FatFS file helpers. Definitions live in FileSystem.c. */
#include <stdbool.h>
#include "ff.h"

bool MountSdCard(void);
bool SelectActiveDrive(void);
bool OpenFile(FIL *file, const char *filename);
void CloseFile(FIL *file);
unsigned int ReadFile(FIL *file, void *buffer, unsigned int bytesToRead);
unsigned int WriteFile(FIL *file, void *buffer, unsigned int bytesToWrite);
void UnMountSdCard(void);
bool PathOrFileExists(const char *path);
void CreatePathDirectories(const char *path);

#endif /* __FILESYSTEM_H_ */
