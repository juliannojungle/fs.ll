# AGENTS.md

Working notes for AI agents and new contributors on **fs.ll**.

## Ground rule: everything written to this repository is in English

This is not a preference, it is a hard rule, and it holds regardless of the language being
spoken in the chat.

- **English only, everywhere in the repository**: code, identifiers, string literals,
  comments, commit messages, `README.md`, this file, and any other document. Not a single
  variable or comment in another language.
- **The chat is separate.** Julianno, the lead dev, prefers to talk to the agent in
  pt-BR. That says nothing about what gets written to disk: the conversation can be in
  pt-BR while every byte committed stays in English.
- **If you find non-English text anywhere outside the chat, stop and ask the dev** whether
  it is intentional or should be corrected. Do not translate it silently, and do not assume
  it is a mistake.

Known and intentional exception, confirmed by the lead dev: `sample/sdcard/st_benedict.txt`
holds a Latin prayer, and **it must be kept exactly as it is**. It is fixture content inside
the sample disk image, not documentation. Do not translate it, replace it, or "normalize" it
to English. The same text also lives inside `sample/sdcard.img`, which is what
`src/Sample.c` reads and prints.

## 1. What this project is

fs.ll ("low level file system") is a small C library that gives an application a simple,
platform-agnostic API to read and write files on an SD card formatted with FAT. It wraps
[FatFs](http://elm-chan.org/fsw/ff/) and supplies the platform-specific pieces FatFs needs.

It is a **library meant to be consumed by other projects**, not an application. The code
under `src/lib` is the library; `src/Sample.c` is only a usage example, and the root
`CMakeLists.txt` exists to build that example.

Supported platforms: `Simulator` (Linux/desktop, and Windows via WSL), `RP2040`
(Raspberry Pi Pico, via pico-sdk) and `ESP32` (via ESP-IDF).

The design goal, stated plainly: give access to the file system of **one single physical SD
card wired to the hardware**, in a way that is agnostic to the platform, so that the same
application code written against this library compiles and runs unchanged on the RP2040, on
the ESP32, and on a Linux simulator. The Simulator is not a separate feature — it is a
stand-in for the physical card, with `sample/sdcard.img` playing the role of the hardware.

So the single-card, single-volume scope is a deliberate design decision, not an accidental
limitation. A change that widens it is a change of intent, not a fix.

Thread safety is a different matter and must not be read as part of that decision. The
library is **not** thread-safe today, and that is not a settled choice: it is an open item,
because the main application that will consume fs.ll runs two active threads that both read
and write the card. See §12 before writing anything that assumes either answer.

## 2. Repository layout

```
CMakeLists.txt                     builds src/Sample.c (the example), per platform
fs.ll.cmake                        the library's build contract; copied into consumer projects
AGENTS.md                          this file
sample/sdcard/                     content that lives inside the disk image
sample/sdcard.img                  1 MiB FAT image used by the Simulator platform
src/Sample.c                       usage example / manual test program
src/lib/FileSystem.{c,h}           the public API (platform independent)
src/lib/Platform/<Platform>/       one folder per platform, same file names in each:
    HAL.{c,h}                        Delay(), STDIOInitAll()
    HALConfig.h                      pin/peripheral/image configuration macros
    RTC.{c,h}                        RTCInitialize() and FatFs's get_fattime()
    DiskIO.c                         FatFs disk_* callbacks + SDCardInit()
    CMakeLists.txt                   ESP32 only: ESP-IDF component registration
src/Dependency/fatfs/              git submodule (a fork, see §8)
src/Dependency/fatfs.ffconf_patch.cmake   rewrites ffconf.h at configure time
src/Dependency/pico_sdk_import.cmake      stock pico-sdk locator, used by the RP2040 build
```

## 3. Architecture

Three layers, with FatFs sitting in the middle:

```
application (src/Sample.c, or a consumer project)
        |  FileSystem.h
FileSystem.c  ........................ platform independent
        |  f_mount, f_open, f_read, ...
FatFs (src/Dependency/fatfs)
        |  disk_initialize/read/write/ioctl, get_fattime
DiskIO.c, RTC.c, HAL.c  ............. one implementation per platform
        |
SD card over SPI, or a file-backed image on the Simulator
```

Porting to a new platform means adding one folder under `src/lib/Platform/` with the same
five file names. Nothing else changes: `fs.ll.cmake` derives the sources from
`PLATFORM_NAME` (§6), so a new folder is picked up automatically.

The three seams between the layers:

- **`disk_initialize` / `disk_status` / `disk_read` / `disk_write` / `disk_ioctl`** in
  `DiskIO.c` are called by FatFs. All three platforms reject any `pdrv != 0`, i.e. exactly
  one physical drive exists. Sector size is 512 everywhere.
- **`get_fattime`** in `RTC.c` is called by FatFs to timestamp directory entries. It must
  not be `static`: FatFs declares it `extern` in `ff.h`, and it is defined in exactly one
  translation unit per program.
- **`SDCardInit`** in `DiskIO.c` is called by `MountSdCard`, and is the only platform
  function the platform-independent layer calls directly. It is forward-declared inside
  `FileSystem.c` rather than exposed in a header (§5).

Platform differences that matter:

| | Simulator | RP2040 | ESP32 |
|---|---|---|---|
| backing store | `fopen` on `SD_DISK_IMAGE` | SD over SPI, bit-level command layer | SD over SPI via `driver/spi_master` |
| card detect | none | `SD_DETECT_PIN`, LOW = present, pull-up | same, plus a GPIO ISR |
| `Delay` | `sleep(ms / 1000)` | `sleep_ms` | `vTaskDelay` |
| `STDIOInitAll` | no-op | `stdio_init_all`, CRLF translation off | no-op (ESP-IDF console does it) |
| time source | `localtime()`, real time | hardware RTC seeded to 2025-01-01 | system time set to 2025-01-01 UTC |
| entry point | `main()` | `main()` | `app_main()` (`ESP_PLATFORM` is defined) |

The card-detect polarity (LOW = card present, with a pull-up) is **validated on hardware**,
not an assumption. Do not "correct" it.

The RP2040 `DiskIO.c` command layer (CMD0/CMD8/ACMD41, SDHC block addressing) was ported
from `no-OS-FatFS-SD-SPI-RPi-Pico`; the ESP32 one follows the same structure.

## 4. Public API and how it is used

Everything the consumer needs is in `FileSystem.h`:

```c
bool MountSdCard(void);                  /* SDCardInit + f_mount("0:") */
bool SelectActiveDrive(void);            /* f_chdrive("0:") */
bool OpenFile(FIL *file, const char *filename);
void CloseFile(FIL *file);
unsigned int ReadFile(FIL *file, void *buffer, unsigned int bytesToRead);
unsigned int WriteFile(FIL *file, void *buffer, unsigned int bytesToWrite);
void UnMountSdCard(void);
bool PathOrFileExists(const char *path);
void CreatePathDirectories(const char *path);
```

The consumer owns the `FIL` object; `FileSystem.c` owns the single `static FATFS fatfs`.
`FRESULT` codes are translated to English messages by the internal `ResultMessage()` and
printed with `printf`, so the API surface is boolean instead of error-coded.

Canonical flow, from `src/Sample.c`:

```c
STDIOInitAll();
RTCInitialize();

FIL file;
if (MountSdCard() && SelectActiveDrive() && OpenFile(&file, "st_benedict.txt")) {
    char buffer[1024];
    UINT bytesRead = ReadFile(&file, buffer, sizeof(buffer) - 1);
    buffer[bytesRead] = '\0';       /* the library never NUL-terminates for you */
    printf("%s", buffer);
    CloseFile(&file);
}

CreatePathDirectories("tmp/01/02/03/test.txt");   /* creates dirs, not the file */
/* ... OpenFile / WriteFile / CloseFile ... */

UnMountSdCard();
```

Semantics that are easy to get wrong:

- `MountSdCard` and `SelectActiveDrive` are separate calls, and both are required before
  using relative paths. `SelectActiveDrive` needs `FF_FS_RPATH >= 1` and `FF_VOLUMES >= 2`
  in `ffconf.h` (§8).
- `OpenFile` uses `FA_OPEN_ALWAYS | FA_READ | FA_WRITE`: it creates the file when missing,
  opens read/write, positions at offset 0 and **does not truncate**. Writing a shorter
  string over a longer file leaves the old tail behind.
- `ReadFile` / `WriteFile` discard the `FRESULT` and return only the byte count, so a
  hard I/O error is indistinguishable from a short read at end of file.
- `SelectActiveDrive` and `OpenFile` call `f_unmount` on failure. Failing to open a file
  therefore tears the volume down, and the caller must mount again before retrying.
- `CreatePathDirectories` takes a *file* path and creates the directories leading to it.
  It ignores `f_mkdir` errors and truncates the path at 256 bytes.

## 5. Code conventions

These are inferred from the existing code. Follow them; do not "modernize" them silently.

- **Function declarations belong in the caller or in the platform header, not scattered
  into shared headers.** `FileSystem.c` forward-declares `SDCardInit()` locally, with a
  comment explaining why, instead of introducing a `DiskIO.h`.
- **PascalCase for this project's own functions** (`MountSdCard`, `SDCardInit`,
  `RTCInitialize`), lowercase snake_case only where FatFs dictates the name
  (`disk_read`, `get_fattime`).
- **Platform folders expose identical file and function names.** The build selects a
  folder; the code never has `#ifdef PLATFORM_X` branches. The single exception is the
  entry point in `src/Sample.c`, which switches on `ESP_PLATFORM`.
- **`HALConfig.h` holds hardware/environment configuration only**, as `#ifndef`-guarded
  macros so a consumer can override them from the build. FatFs-level constants that are
  identical on every platform (such as `SD_DRIVE "0:"`) stay in `FileSystem.c`.
- **Comments explain *why***, and are used generously where a decision looks arbitrary
  (why `get_fattime` is not static, why `SD_SPI` must match the pins). In English, always
  — see the ground rule at the top of this file.
- Some `.c` files carry include guards (`#ifndef __FILESYSTEM_`, `#ifndef _HAL_C_`).
  That is inherited style, harmless, and inconsistently applied. Leave it alone.
- `snake_case` locals and `PascalCase` struct members appear side by side in the ported
  `DiskIO.c` files. Match the surrounding file rather than the project.

Note: a comment in `FileSystem.c` refers to "AGENTS.md §5". That reference predates this
file (fs.ll was split out of a larger project, and `FileSystem.c` came with it), but §5
here is deliberately the conventions section, so it now resolves correctly.

## 6. Build system

There is no `add_library` anywhere. The library publishes two list variables that the
consumer feeds into its own target:

- `SOURCES` — appended with `FileSystem.c`, the three platform files and the three FatFs
  sources.
- `INCLUDE_DIRS` — appended with `src/lib`, the platform folder and the FatFs source dir.

Both are *appended to*, never overwritten, so a consumer can accumulate several libraries
that follow this same architecture into one target.

### `fs.ll.cmake`

This is the entry point for consumers. **A consumer copies this one file into its own
repository** and includes it after `project()`. Contract:

| variable | role |
|---|---|
| `FS_LL_PATH` | in/out. Root of the fs.ll checkout. Accepted as a normal variable or an environment variable; relative paths resolve against `CMAKE_SOURCE_DIR`. Defaults to a `fs.ll` folder next to the copied file. Ends up in the cache. |
| `PLATFORM_NAME` | in. `Simulator` (default), `RP2040` or `ESP32`. Selects the platform folder. |
| `FS_LL_PLATFORM_DIR` | out. The resolved platform folder. |

Resolution is two steps: default the path, then check the sentinel file
`src/lib/FileSystem.c`. If the sentinel is missing, the directory is populated with
`git clone --branch main --depth 1 --recurse-submodules --shallow-submodules` at configure
time via `execute_process`. Consequences to be aware of:

- Because `FS_LL_PATH` is cached with `FORCE`, several submodules of the same project can
  each carry their own copy of `fs.ll.cmake` and still share a single checkout: the first
  one to resolve it wins, the rest reuse the cached path. `list(REMOVE_DUPLICATES)` on
  `SOURCES`/`INCLUDE_DIRS` keeps the second include from duplicating sources.
- The download is deliberately **not** `FetchContent`. FetchContent pulls dependencies
  into the build tree and insists on `add_subdirectory`, which would try to build fs.ll's
  own example project; and the checkout has to live at a path the user chooses, outside
  the build tree, so it can be shared. `FetchContent_Populate` is also deprecated
  (policy `CMP0169`). A plain `git clone` avoids all three problems.
- All variables are prefixed `FS_LL_` on purpose. Sibling libraries following this same
  architecture get included into the same `CMakeLists.txt`, so generic names would
  collide. `PLATFORM_NAME` is the one intentionally shared input.

### Root `CMakeLists.txt` (builds the example)

Sets `FS_LL_PATH` to `CMAKE_SOURCE_DIR` up front, because for an in-tree build this
repository *is* the fs.ll root and nothing should be downloaded. Then it branches on
`PLATFORM_NAME`:

- `Simulator`: plain `project()`, include `fs.ll.cmake`, prepend `src/Sample.c` to
  `SOURCES`, `add_executable`.
- `RP2040`: defaults `PICO_SDK_PATH` to `~/pico-sdk`, includes
  `src/Dependency/pico_sdk_import.cmake`, calls `pico_sdk_init()`, links
  `pico_stdlib hardware_spi hardware_gpio hardware_adc hardware_rtc`, enables stdio over
  USB (UART off) and adds the uf2/map/hex outputs.
- `ESP32`: does *not* include `fs.ll.cmake` itself. It points `EXTRA_COMPONENT_DIRS` at
  `src/lib/Platform/ESP32` and hands the build to ESP-IDF, whose component
  `CMakeLists.txt` there includes `fs.ll.cmake` and calls `idf_component_register` with
  `REQUIRES driver esp_system`. That component file also sets `FS_LL_PATH` from its own
  location as a fallback, because ESP-IDF may process the component in a scope where
  neither `CMAKE_SOURCE_DIR` nor an inherited `FS_LL_PATH` is available; without it the
  default path rule would clone fs.ll into the repository itself.

`include_directories()` is called after `add_executable()` in the root file. This works
(directory-level include dirs are resolved at generate time) — it is not a bug to fix.

## 7. Building and running

`CMAKE_EXPORT_COMPILE_COMMANDS` is on and `.clangd` reads `build/compile_commands.json`,
so **build into `build/`** if you want working code intelligence. `.clangd` also strips
the ARM flags (`-mthumb`, `-mcpu`, ...) so the desktop clangd can parse the embedded
builds, and defines `DEBUGMSGS`.

`.vscode/tasks.json` has ready-made tasks; they hardcode `~/fs.ll` as the working
directory. The equivalent commands:

```bash
# Simulator (desktop)
cmake -B build -DPLATFORM_NAME=Simulator && cmake --build build
./build/fs.ll                 # run from the repo root, see below

# RP2040 (needs pico-sdk at ~/pico-sdk or PICO_SDK_PATH)
cmake -B build -DPLATFORM_NAME=RP2040 && cmake --build build

# ESP32 (needs ESP-IDF exported in the shell)
source ~/esp-idf/export.sh && idf.py -DPLATFORM_NAME=ESP32 build
```

The Simulator reads and writes `sample/sdcard.img` through the relative path in
`SD_DISK_IMAGE`, so **the binary must be started from the repository root**, not from
`build/`. The image is 1 MiB; `sample/sdcard/` holds the files that live inside it.
Running the example mutates the image (it creates `tmp/01/02/03/test.txt`), which shows
up as a modified binary file in `git status`.

A script that builds `sdcard.img` from `sample/sdcard/` exists in the project fs.ll was
split out of, but has **not been migrated here yet** (it comes with more environment-setup
files, deferred on purpose). Until it lands, do not invent a replacement and do not assume
the committed image can be regenerated locally: treat `sample/sdcard.img` as a fixture, and
restore it with `git checkout -- sample/sdcard.img` after a run dirties it.

## 8. The FatFs dependency

`src/Dependency/fatfs` is a git submodule pointing at the fork
`https://github.com/juliannojungle/fatfs.git`, declared `shallow = true` and
`ignore = all`. The path is also listed in `.gitignore`. Only three of its sources are
compiled: `ff.c`, `ffsystem.c`, `ffunicode.c`.

`src/Dependency/fatfs.ffconf_patch.cmake` is included at the end of `fs.ll.cmake` and
rewrites the submodule's `ffconf.h` in place, by regex, at configure time:

| setting | stock | patched | why |
|---|---|---|---|
| `FF_FS_RPATH` | 0 | 1 | enables `f_chdrive`/`f_chdir`, i.e. relative paths |
| `FF_VOLUMES` | 1 | 2 | `f_chdrive` and the drive-prefix code in `ff.c` only compile under `#if FF_VOLUMES >= 2` |
| `FF_CODE_PAGE` | 932 | 437 | US instead of Japanese |
| `FF_USE_LFN` | 0 | 2 | long file names, working buffer on the stack |

`FF_VOLUMES` is the number of *logical* volumes (it sizes `static FATFS *FatFs[FF_VOLUMES]`
in `ff.c` and decides which drive prefixes such as `"0:"` are valid). It is not the number
of physical drives, and raising it to 2 costs one pointer, not a second `FATFS`. The
project still mounts only `"0:"`; the bump exists to enable the `f_chdrive` code path.

Two consequences of patching in place: the submodule's working tree is dirty after a
configure, and every project sharing one `FS_LL_PATH` shares this FatFs configuration.

**No change inside `src/Dependency/fatfs` is ever to be versioned from this repository.**
That is exactly why the path is in `.gitignore` and the submodule is declared `ignore = all`
— to hide the dirt the patch creates. Treat the submodule as read-only build input: never
`git add` anything under it, never commit the patched `ffconf.h`, and never bump the
submodule pointer as a side effect of another change. Configuration changes belong in
`fatfs.ffconf_patch.cmake`; upstream changes belong in the fork.

## 9. Traps and sharp edges

Known, unfixed, and easy to trip over. Do not "fix" these as a drive-by; they are recorded
here so a change to them is a deliberate decision.

- `CreatePathDirectories("")` reads `tmp[len - 1]` with `len == 0`, i.e. out of bounds.
- `CreatePathDirectories` strips a trailing `/`, so for `"a/b/"` the directory `b` is not
  created. It also starts scanning at `tmp + 1`, so a leading `/` is not a separator.
- Simulator `Delay(500)` sleeps 0 seconds, because it is `sleep(ms / 1000)`.
- Simulator `RTC.c` declares its own `typedef unsigned int DWORD;` instead of including
  `ff.h`. It compiles only because `ff.h` is not included in that file.
- Simulator `HALConfig.h` defines SPI pins that nothing reads; only `SD_DISK_IMAGE` is used.
- `Delay` takes `unsigned int` on Simulator/RP2040 and `uint32_t` on ESP32.
- The single `static FATFS fatfs` means one mounted volume per program, and no reentrancy
  (§12 — this one is an open decision, not a settled trap).
- `PathOrFileExists` is `f_stat`, which cannot stat the root directory itself.
- Read buffers are the caller's responsibility, including NUL termination; `src/Sample.c`
  reads `sizeof(buffer) - 1` on purpose.
- `src/Sample.c` carries on after a failed mount: its second half is not guarded, so
  running it with no reachable card prints `MountSdCard error: SDCardInit failed` followed
  by two `The volume has no work area` (`FR_NOT_ENABLED`) errors from `OpenFile`.

## 10. Not verified

Claims in this file about the `Simulator` platform were checked by configuring, building
and running the example, including the working-directory dependency and the mutation of
the disk image. The `fs.ll.cmake` contract in §6 was exercised in all its paths: in-tree
build, external consumer with `FS_LL_PATH` given, external consumer with nothing set
(which cloned), reconfigure over a populated directory, and two copies of the file sharing
one checkout.

The `RP2040` and `ESP32` builds were **not** executed (they need pico-sdk and ESP-IDF), so
statements about them come from reading the code and the build files. The ESP-IDF scope
fallback described in §6 is defensive: the failure mode it prevents was reasoned about,
not reproduced.

## 11. Still to come from the parent project

fs.ll was split out of a larger project so that the file system layer could be reused as a
library. Some material still lives over there and is expected to land here eventually:

- the script that builds `sample/sdcard.img` from `sample/sdcard/`, together with the rest
  of the environment-setup files it depends on (§7);
- pinout documentation, the list of compatible devices, and related hardware notes.

Until then, absence of documentation here does not mean the information does not exist. If
a hardware detail is missing, ask rather than deriving it from the code, and never guess a
pinout: `HALConfig.h` holds the values in use, and the card-detect polarity there is
hardware-validated (§3). The occasional stale cross-reference (such as the `AGENTS.md §5`
comment in `FileSystem.c`, see §5) also comes from that split.

## 12. Open decision: thread safety

**Status: undecided. Do not implement anything for this on your own initiative.**

The library is not thread-safe today, and that is not the result of a design choice — it is
simply the state the code is in. It matters because the main application that will consume
fs.ll runs **two active threads**, both reading and writing the card, and in some cases
wants the same file open for writing in one thread while the other reads it.

### What the code does today

- `FF_FS_REENTRANT` is `0` in `ffconf.h`, so the entire `ff_mutex_*` block of `ffsystem.c`
  is compiled out. Nothing serializes two tasks calling `f_*` on the same volume.
- `FF_FS_LOCK` is `0`, so FatFs does not police duplicated open, nor rename/delete of an
  open object.
- `FileSystem.c` holds a single `static FATFS fatfs`, and `f_mount` is never thread-safe in
  FatFs regardless of configuration.
- `DiskIO.c` keeps mutable statics (`sdState`, `spiHandle`, `busInited`, `diskFile`) and
  shares one SPI bus. `SdAcquire`/`SdRelease` only toggle chip select; they are not locks.
- The card-detect ISR (RP2040 and ESP32) writes `sdState.Initialized = false` while the main
  context may be mid-operation. It is a single `bool` write, benign in practice, but it is a
  genuine concurrent writer.
- `FF_USE_LFN` is `2`, i.e. the LFN working buffer lives on the stack. That part is already
  reentrancy-friendly; the value that would break it is `1` (static buffer).
- Nothing in the library starts a thread. On the ESP32 the code runs inside the `app_main`
  task under FreeRTOS and `Delay` is `vTaskDelay`, which yields the CPU — so a context
  switch during a disk operation is not hypothetical there.

### What FatFs allows

From its own appnote (`src/Dependency/fatfs/documents/doc/appnote.html`, sections
"Re-entrancy" and "Duplicated File Open"):

- Two tasks on the same volume are not thread-safe by default. `FF_FS_REENTRANT = 1` plus
  platform implementations of `ff_mutex_create/delete/take/give` makes each filesystem
  object exclusive, with a `FF_FS_TIMEOUT` after which calls fail with `FR_TIMEOUT`.
- `f_mount` and `f_mkfs` remain non-thread-safe even then, so mounting has to happen before
  the threads that use the volume start.
- Duplicated open of a file is permitted **only when every open is read mode**. Duplicated
  open with any write mode is always prohibited, and an open file must not be renamed or
  deleted. Violating this can corrupt data.
- `FF_FS_LOCK > 0` does not lift that restriction. It makes FatFs reject the violation with
  `FR_LOCKED` instead of corrupting the volume, and it caps how many open objects are
  tracked at once (`FR_TOO_MANY_OPEN_FILES` beyond that).
- The FatFs mutex covers the filesystem object only, not the device layer. The `disk_*`
  functions must be thread-safe on their own if the API is re-entered.

So the specific case of one thread writing a file while another reads that same file is
**not supported by FatFs at any configuration**. Reaching it requires an application-level
design: one shared `FIL` guarded by a mutex (keeping in mind a `FIL` carries a single file
pointer, so seek and I/O have to be locked together), or a writer/reader handoff, or an
in-RAM buffer between the two threads.

### If this gets picked up

`FF_FS_REENTRANT` and `FF_FS_LOCK` are set by `src/Dependency/fatfs.ffconf_patch.cmake`
(§8). The `ff_mutex_*` implementations would become new per-platform code (`ffsystem.c`
carries samples for several OSs). The `DiskIO.c` statics and the SPI bus would need their
own protection, since the FatFs mutex does not reach the device layer. Mount would have to
be pinned to program start, before the worker threads exist.

There is a strong hint that little or nothing needs to change on this side: the parent
application already has a **singleton `dataManager`** that centralizes card reads and writes
between its two threads. If every access really funnels through that one object, the
serialization point already exists above this library, which is precisely the
application-level design the FatFs rules above call for. That has not been verified from
here, and it is the first thing to check when the topic is picked up.

Julianno will drive this decision from the parent application, where the agent has the full
context on how the two threads actually use the card. Ask before changing anything here.
