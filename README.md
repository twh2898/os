# os

~~Following the tutorials under https://github.com/cfenollosa/os-tutorial~~

For current specs see [memory.md](memory.md) and ~~[filesystem.md](filesystem.md)~~

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
- [ ] Paging
  - [ ] Load user space application
- [ ] Ring 3
  - [ ] Kernel Service Calls
- [ ] Date and Time
- [ ] Optional don't disable interrupts during interrupt (nested interrupts)
- [ ] Better key event buffer (with mods) (maybe in addition to char buffer)
- [x] Change stdlib names with k prefix for namespace during testing
- [ ] Optimize disk read to check if area already in buffer

### Long Term

- [ ] Switch to 2d graphics mode
- [ ] Virtual memory pages / memory paging
- [ ] Memory permissions (eg. stack can't exec, code can't write)
- [ ] User level applications (might need 3rd level kernel from filesystem)
- [ ] Testing
- [ ] FAT or EXT2 filesystem driver
- [ ] Audio Driver
- [x] 64 bit support printf (needs libgcc)

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

## Boot Stages

### BIOS

1. Load boot sector into memory at 0x7c00

### Stage 1

1. Store boot drive id
2. Setup stage 1 stack at 0x7bff
3. Read memory map to 0x500
4. Read stage 2 from boot drive
5. Setup GDT
6. Switch to protected mode

### Stage 2

1. Load VGA driver
   1. Clear screen
   2. Print hello
2. Setup ISR
3. Setup IRQ
4. Load RAM driver
   1. Init malloc
   2. Setup Paging
5. ??? - Does anything need to move after paging starts?
6. Load ATA driver
   1. Mount fs
   2. Read kernel into page directory
7. Jump to kernel

### Stage 3 - OS

1. Load drivers
   1. VGA / 2d graphics
   2. Keyboard
   3. Disk
   4. Filesystem
   5. etc.
2. Mount os drive / partition? (does this happen in stage 2?)
3. Setup kernel service calls
4. Begin user space applications loop
   1. Create page directory
   2. Load elf binary
   3. Switch to Ring 3
5. Shell

## Setup

### Format drive.img

After creating the drive image with `make drive.img` you can format and mount it
using the following.

Setup file as drive

```sh
modprobe nbd max_part=63
qemu-nbd -c /dev/nbd0 drive.img
```

Format drive

```sh
fdisk /dev/nbd0
mkfs.fat -F32 /dev/nbd0p1
```

Fdisk commands should be `onp1  w`

Mount the drive

```sh
mount /dev/nbd0p1 drive
```

Unmount the drive

```sh
umount drive
qemu-nbd -d /dev/nbd0
rmmod nbd
```
