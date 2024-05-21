# os

Following the tutorials under https://github.com/cfenollosa/os-tutorial

## Goals

The OS will be finished when all of the following are implemented.

- [ ] Able to get and draw basic http web page
- [ ] Compiler for at least Assembly or C
- [ ] Playable Minecraft Clone

## ToDo

- [ ] Kernel stats (stats command returns # disk io, keys, mounts, etc.)
- [ ] Detect stack overflows
- [ ] fs
  - [ ] Finish implementing fs functions
  - [ ] Implement file io
- [x] basic malloc (linked list)
- [ ] malloc
  - [x] Actually manage memory
  - [ ] Need to find end of kernel to not overflow at runtime (for malloc start)
  - [x] Detect max memory for malloc
  - [x] Do that fancy memory map
- [ ] Date and Time
- [ ] Optional don't disable interrupts during interrupt
- [ ] Better key event buffer (with mods) (maybe in addition to char buffer)
- [ ] Change stdlib names with k prefix for namespace during testing

### Long Term

- [ ] Switch to 2d graphics mode
- [ ] Virtual memory pages / memory paging
- [ ] Memory permissions (eg. stack can't exec, code can't write)
- [ ] User level applications (might need 3rd level kernel from filesystem)
- [ ] Testing
- [ ] FAT or EXT2 filesystem driver
- [ ] Audio Driver
- [ ] 64 bit support printf (needs libgcc)

### Long Long Term

- [ ] Graphics card driver (pseudo opengl)
- [ ] Threading / multi-process
- [ ] (Maybe) Multiboot or GRUB?

## Completed

- Read extended kernel form floppy
- Switch to protected mode
- some stdio.h
- printf
- some string.h
- keyboard driver
- ata driver
- terminal with command parsing
- circular buffer + tests (builtin, needs separating)
