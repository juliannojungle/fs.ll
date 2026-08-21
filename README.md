# fs.ll

> 🦖 **Part of [dot-ll-collection](https://github.com/topics/dot-ll-collection)**

> ⚠️ **This project is under active development. The documentation is growing along the project as it's a work-in-progress.**

This is a lightweight, bare-metal file system library for embedded systems.

Provides a simple, platform-agnostic API to read and write files on an SD card formatted with FAT. It wraps FatFs and supplies the platform-specific pieces FatFs needs.

## Premise

Give access to the file system of **one single physical SD card wired to the hardware**, in a way that is agnostic to the platform, so that the same application code written against this library compiles and runs unchanged on the RP2040, on
the ESP32, and on a Linux simulator. The Simulator is not a separate feature — it is a stand-in for the physical card, with `sample/sdcard.img` playing the role of the hardware.

Currently supports **RP2040** (Raspberry Pi Pico), **ESP32**, and a **Simulator** (native desktop via SDL2) from a single codebase.
