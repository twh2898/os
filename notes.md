# Notes and ToDos for active and planned future work

## Creation of Page Tables

**TODO** how are page tables added to the virtual space?

Idk if the kernel initializes all of the entries of the final page table or only
the first and last.

The first and last page tables are setup but nothing in between. Does the paging
allocator create new page tables if needed?

### Tasks

- [ ] Check if page allocator checks if the first page table is full
- [ ] Check if page allocator will create and add new page tables to the page directory

lol, it's all TODO comments


# Completed

## Physical Allocator

**TODO** how to access region bitmask in paging mode?

I wrote the physical allocator before the paging allocator. So all operations
expect direct access (identity mapping) of the region bitmask page.

### Ideas

1. Have a dedicated virtual address for all possible region bitmask pages
   - Would require extra lookup from region table for physical addresses of each
     page (ie. cant just use bitmask address + page index)
2. ...

### Tasks

- [x] It looks like I might have already accounted for this in the virtual
  address space table. I need to check if this has been implemented.
  - The bitmasks are already mapped to their virtual space starting at 0x9f000
  - ram.h functions operate in virtual address space (except `init_ram` and `create_bitmask`)
- [x] Finish commenting `map_virt_page_dir` in kernel.c
