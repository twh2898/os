# Boot Stages

This describes the planned stages and steps, it does not represent the current
implementation or progress.

## BIOS

The BIOS firmware does a lot more than this, only the steps to launch the kernel
are included.

1. Power On Self Test (POST)
2. Load boot sector (512 Bytes) into memory at 0x7c00
3. Jump to 0x7c00

## Stage 1 - Boot

Execution of the first 512 bytes "boot sector".

1. Store boot drive id
2. Setup stage 1 stack at 0x06fff
3. Read memory map to 0x500
4. Read stage 2 from boot drive to 0x07e00
5. Setup GDT
7. Switch to protected mode
8. Jump to 0x07e00

## Stage 2 - Kernel

Kernel in protected mode setting up system for user space applications.

1. Load VGA driver
   1. Clear screen
   2. Print hello
2. Setup ISR and IDT
   1. Init timer
   2. Init keyboard
   3. Init ata
   4. Init rtc
3. Setup Memory Allocator
   1. Physical Memory
   2. Virtual Memory (paging)
4. Enable Paging
5. Load ATA driver
   1. Mount fs
   2. Read kernel into page directory
6. Start kernel

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
