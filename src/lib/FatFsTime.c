#include "ff.h"
#include "HAL.h"

/**
 * Provides the timestamp FatFS puts on directory entries. Called by FatFS when
 * FF_FS_NORTC == 0.
 *
 * NOT static: FatFS declares get_fattime() as extern in ff.h, so it needs exactly
 * one definition across the program, and this is it.
 *
 * Platform independent because reading the clock is hal.ll's job. It used to be
 * three copies, one per platform, differing only in how they reached the clock.
 */
DWORD get_fattime(void) {
    DateTime now;
    RTCGetDateTime(&now);

    return ((DWORD)(now.Year - 1980) << 25)
         | ((DWORD)now.Month << 21)
         | ((DWORD)now.Day << 16)
         | ((DWORD)now.Hour << 11)
         | ((DWORD)now.Min << 5)
         | ((DWORD)(now.Sec / 2));
}
