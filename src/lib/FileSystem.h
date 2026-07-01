#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_

/* Public interface of the SD/FatFS file helpers. Definitions live in FileSystem.c. */
#include <stdbool.h>
#include "ff.h"

bool MountSdCard(void);
bool SelectActiveDrive(void);
bool OpenFile(FIL *file, const char *filename);
void CloseFile(FIL *file);
void UnMountSdCard(void);

#endif /* __FILESYSTEM_H_ */
