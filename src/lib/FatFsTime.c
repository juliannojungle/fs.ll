/*
    Pedal.guru is an open-source software
    for cycle computers based on DIY hardware (primarily Raspberry Pi).
    Copyright (C) 2022, Julianno F. C. Silva (@juliannojungle)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.
*/

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
