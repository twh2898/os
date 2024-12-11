# os

~~Following the tutorials under https://github.com/cfenollosa/os-tutorial~~

Active and planned work is tracked in [notes.md](notes.md)

For current specs see

- [memory.md](design/memory.md)
- [boot.md](design/boot.md)
- ~~[filesystem.md](design/filesystem.md)~~

## Goals

The OS will be finished when all of the following are implemented.

1. Able to get and draw basic http web page
2. Compiler for at least Assembly or C
3. Playable Minecraft Clone

The following lists show the planned development stages and planned tasks to
achieve these goals.

### Near Near Term

- [x] Read extended kernel form floppy
- [x] Switch to protected mode
- [x] some stdio.h
- [x] printf
- [x] some string.h
- [x] keyboard driver
- [x] ata driver
- [x] terminal with command parsing
- [x] circular buffer + tests (builtin, needs separating)

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
- [x] Load user space application
- [ ] Ring 3
  - [x] Kernel Service Calls
- [ ] Date and Time
- [ ] Optional don't disable interrupts during interrupt (nested interrupts)
- [ ] Better key event buffer (with mods) (maybe in addition to char buffer)
- [x] Change stdlib names with k prefix for namespace during testing
- [ ] Optimize disk read to check if area already in buffer

### Long Term

These tasks will be broken down further after completion of Near Term, when
there is a better foundation and design to work with.

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

These tasks will be broken down further after completion of Long Term, when
there is enough support for their development.

- [ ] Graphics card driver (pseudo opengl)
- [ ] Threading / multi-process
- [ ] (Maybe) Multiboot or GRUB?

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
