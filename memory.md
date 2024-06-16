# Memory Map

## Real Mode

See https://wiki.osdev.org/Memory_Map_(x86) for reserved BIOS memory.

| start  | end     | size      | description                 |
| ------ | ------- | --------- | --------------------------- |
| 0x0000 | 0x4ff   | 1.25 KiB  | Unusable                    |
| 0x0500 | 0x00fff | 1.25 KiB  | Boot Parameters             |
| 0x1000 | 0x06fff | 24 KiB    | Stack (real mode), top down |
| 0x7000 | 0x07bff | 3 KiB     | Unused                      |
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

| start   | end     | size      | description                                   |
| ------- | ------- | --------- | --------------------------------------------- |
| 0x00000 | 0x004ff | 1.25 KiB  | Unused                                        |
| 0x00500 | 0x00fff | 2.75 KiB  | Boot Parameters                               |
| 0x01000 | 0x01fff | 4 KiB     | Page Directory                                |
| 0x02000 | 0x02fff | 4 KiB     | First Page Table (virtual address 0xffc00000) |
| 0x03000 | 0x06fff | 16 KiB    | Stack                                         |
| 0x07000 | 0x07bff | 3 KiB     | Unused                                        |
| 0x07c00 | 0x07dff | 512 bytes | Unused *GDT here                              |
| 0x07e00 | 0x9efff | 604.5 KiB | Kernel (second stage)                         |

> [!IMPORTANT] Kernel Size in Protected Mode
> Reserved memory in protected mode starts at 0x9fc00 while real mode free area
> ends at 0x9ffff

### Virtual Address Space

- The first 10 pages are identity mapped. The heap starts with virtual page 11.
- The address 0x1000 will always point to the active page directory.
- ~~The address 0x2000 will always point to the active page table~~
- The address 0x2000 will not be present / mapped to anything
- _0x400000 is the first virtual address of the second page table_
- 0xffc00000 is the first virtual address of the last page table
  - This goes up to 0xffffffff as the last address in all of virtual space

TODO map area for ram region bitmask entries

| start      | end        | pages   | description                               |
| ---------- | ---------- | ------- | ----------------------------------------- |
| 0x00000000 | 0x00000fff | 0x00001 | null page (not present)                   |
| 0x00001000 | 0x00001fff | 0x00001 | Page Directory                            |
| 0x00002000 | 0x00002fff | 0x00001 | ram region table                          |
| 0x00003000 | 0x00006fff | 0x00004 | Stack                                     |
| 0x00007000 | 0x0009efff | 0x00098 | Kernel (0x0009fbff end of kernel)         |
| 0x0009f000 | 0x0029efff | 0x00200 | ram region bitmasks                       |
| 0x0029f000 | 0xffbfffff | 0xff961 | _free memory_                             |
| 0xffc00000 | 0xffc00fff | 0x00001 | first page table (includes identity map)  |
| 0xffc01000 | 0xffffefff | 0x003fe | all page tables from the active directory |
| 0xfffff000 | 0xffffffff | 0x00001 | last table (last entry is this table)     |

## Memory Allocation

Physical Allocator

- Allocates single pages from each region in order
- Keeps track of free and used pages

Paging Allocator

- Requests pages form the physical allocator to append to page tables as needed
- Keeps track of free pages in the middle and can re-use them

## Physical Allocator

Physical pages are tracked by a region table. The region table has on entry
for each region of free pages where the first page is a bitmask with one bit
per following page.

### Region Table

Regions are used to track all pages. Each region has up to 32768 continuous
pages (128 MiB) where the first page is a bitmask of all free pages in that
region. All regions are tracked by a Region Table which holds up to 512 region
pointers. Each entry in the region table also includes a flag if the region
is present and counts of the total number of pages in the region and number of
free pages in the region.

| start | size | description     |
| ----- | ---- | --------------- |
| 0     | 4    | address + flags |
| 4     | 2    | page count      |
| 8     | 2    | free count      |

>[!IMPORTANT] Remember to mask the address
> The low 12 bits of each address are flags because the address is always page
> aligned. Remember to mask these bits when using the address.

> [!NOTE] Region size
> Each region can be up to 128 MiB (32768 pages)
>- Page size = 4096 (0x1000)
>- Bits per page = Page Size * 8 = 4096 * 8 = 32768 (0x8000)
>- Max pages per region = Bits per page = 32768 (0x8000) (includes bitmask page)
>- Max region size = Max pages per region * Page size = 32768 * 4096 = 134217728
>  (0x8000000) = 128 MiB

#### Flags

| flag | description                                                |
| ---- | ---------------------------------------------------------- |
| 0x1  | Present - this region table entry points to a valid region |

### Region Bitmask

The first page of each region (pointed to by the region table entry) is a
bitmask for the region showing which pages are free. Any bits outside of the
region should be set to 0 (ie. if the region is smaller than 128 MiB, bits for
any pages after the region end should be 0).

> [!IMPORTANT] Bitmask bit 1
> The first bit of the bitmask will always be 0 for the bitmask page itself.

## Paging Allocator

TODO - everything here
