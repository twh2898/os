# os

~~Following the tutorials under https://github.com/cfenollosa/os-tutorial~~

Active and planned work is tracked in [notes.md](notes.md)

For current specs see

- [memory.md](memory.md)
- ~~[boot_stages.md](boot_stages.md)~~
- ~~[filesystem.md](filesystem.md)~~

## Goals

The OS will be finished when all of the following are implemented.

1. Able to get and draw basic http web page
2. Compiler for at least Assembly or C
3. Playable Minecraft Clone

### Near Term

- [ ] Kernel stats (stats command returns # disk io, keys, mounts, etc.)
- [ ] Detect stack overflows
- [ ] fs
  - [ ] Finish implementing fs functions
  - [ ] Implement file io
- [x] basic malloc (linked list)
- [x] malloc
  - [x] Actually manage memory
  - [x] Need to find end of kernel to not overflow at runtime (for malloc start)
  - [x] Detect max memory for malloc
  - [x] Do that fancy memory map
- [x] Paging
  - [x] Setup page dir and table
  - [x] Enter paging
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
- [x] Virtual memory pages / memory paging
- [ ] Memory permissions (eg. stack can't exec, code can't write)
- [ ] User level applications (might need 3rd level kernel from filesystem)
- [ ] Testing
- [ ] FAT or EXT2 filesystem driver
- [ ] Audio Driver
- [x] 64 bit support printf (needs libgcc)
- [ ] Text editor

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

This describes the planned stages and steps, it does not represent the current
implementation or progress.

### BIOS

1. Load boot sector into memory at 0x7c00

### Stage 1

1. Store boot drive id
2. Setup stage 1 stack at 0x06fff
3. Read memory map to 0x500
4. Read stage 2 from boot drive
5. Setup GDT
7. Switch to protected mode

### Stage 2

1. ~~Move boot parameters out of first page~~
   1. ~~Set first page to throw error on access to 0~~
2. Setup ISR and IDT
4. Setup Memory Allocator
   1. Physical Memory
   2. Virtual Memory (paging)
5. Enable Paging
6. Load VGA driver
   1. Clear screen
   2. Print hello
8.  Load ATA driver
   1. Mount fs
   2. Read kernel into page directory
9.  Start kernel

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

```sh
git clone git@github.com:twh2898/os.git
cd os
make setup
```

### Building

```sh
make build
```

### Running

```sh
make run
```

Once running, use the `help` command to see what you can do.

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
