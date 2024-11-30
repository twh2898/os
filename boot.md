# Boot Stages

This describes the planned stages and steps, it does not represent the current
implementation or progress.

## BIOS

1. Load boot sector into memory at 0x7c00

## Stage 1

1. Store boot drive id
2. Setup stage 1 stack at 0x06fff
3. Read memory map to 0x500
4. Read stage 2 from boot drive
5. Setup GDT
7. Switch to protected mode

## Stage 2

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

## Stage 3 - OS

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
