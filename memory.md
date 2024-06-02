# Memory Map

## Real Mode

See https://wiki.osdev.org/Memory_Map_(x86) for reserved BIOS memory.

| start  | end     | size      | description                 |
| ------ | ------- | --------- | --------------------------- |
| 0x0500 | 0x00fff | 1.25 KiB  | Memory Map                  |
| 0x1000 | 0x01fff | 4 KiB     | Page Directory              |
| 0x2000 | 0x02fff | 4 KiB     | Page Table                  |
| 0x3000 | 0x07bff | 4 KiB     | Stack (real mode), top down |
| 0x7c00 | 0x07dff | 512 bytes | Boot Sector                 |
| 0x7e00 | 0x9fbff | 607.5 KiB | Kernel (second stage)       |

> [!IMPORTANT] Kernel Size in Protected Mode
> Reserved memory in protected mode starts at 0x9fc00 while real mode ends at
> 0x9ffff
