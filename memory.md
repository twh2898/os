# Memory Map

## Real Mode

See https://wiki.osdev.org/Memory_Map_(x86) for reserved BIOS memory.

| start  | end     | size      | description                 |
| ------ | ------- | --------- | --------------------------- |
| 0x0500 | 0x00fff | 1.25 KiB  | Boot Parameters             |
| 0x1000 | 0x01fff | 4 KiB     | Page Directory              |
| 0x2000 | 0x02fff | 4 KiB     | Page Table                  |
| 0x3000 | 0x07bff | 18.99 KiB | Stack (real mode), top down |
| 0x7c00 | 0x07dff | 512 bytes | Boot Sector                 |
| 0x7e00 | 0x9fbff | 607.5 KiB | Kernel (second stage)       |

> [!IMPORTANT] Kernel Size in Protected Mode
> Reserved memory in protected mode starts at 0x9fc00 while real mode free area
> ends at 0x9ffff

### Boot Parameters

Boot parameters are set by the bootloader and passed to the second stage kernel
at address 0x500.

| start | size | description        |
| ----- | ---- | ------------------ |
| 0     | 2    | Low Memory Size    |
| 2     | 2    | Memory Entry Count |
| 4     | x    | Memory Map Entries |

#### Memory Entry

There is a single memory entry for each region of memory. It is possible to have
out of order and overlapping regions. See [BIOS Function: INT 0x15, EAX =
0xE820](https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15.2C_EAX_.3D_0xE820)

| start | size | description  |
| ----- | ---- | ------------ |
| 0     | 8    | Base Address |
| 8     | 8    | Length       |
| 16    | 4    | Type         |
| 20    | 4    | Ext          |

Region Type can be one of the following

| Type | Description      | Usable   |
| ---- | ---------------- | -------- |
| 1    | Usable RAM       | Usable   |
| 2    | Reserved         | Reserved |
| 3    | ACPI Reclaimable | Usable   |
| 4    | ACPI NVS         | Reserved |
| 5    | Bad Memory       | Reserved |

## Protected Mode

| start  | end     | size      | description           |
| ------ | ------- | --------- | --------------------- |
| 0x0000 | 0x004ff | 1.25 KiB  | Unused                |
| 0x0500 | 0x00fff | 2.75 KiB  | Boot Parameters       |
| 0x1000 | 0x01fff | 4 KiB     | Page Directory        |
| 0x2000 | 0x02fff | 4 KiB     | Page Table            |
| 0x3000 | 0x07bff | 18.99 KiB | Stack                 |
| 0x7c00 | 0x07dff | 512 bytes | Unused *GDT here      |
| 0x7e00 | 0x9fbff | 607.5 KiB | Kernel (second stage) |
