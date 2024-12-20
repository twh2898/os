# Notes and ToDos for active and planned future work

This is a list of all active and planned work. They can be removed once they are
checked off. Use the task template bellow for larger tasks that need some design
work or require additional information. Those can be moved under the Completed
section once they are finished.

- [ ] Documentation
  - [x] Add design doc for boot stages and move out of readme
  - [x] Update boot stages to be more accurate
  - [ ] Filesystem documentation
  - [x] Move doc files under design folder
  - [x] Clean up goals in readme
    - [x] Remove completed section (make checked boxes in list)
    - [x] Cleanup Goals term lists
  - [ ] System call arch
- [ ] mmu
  - [ ] free interior pages when malloc free's entire virtual page
- [ ] tar
  - [ ] list directories
- [ ] disk
  - [x] ata driver
  - [ ] floppy driver
  - [ ] ram disk driver
- [ ] test (os built in)
  - [ ] circbuff_rpop
  - [ ] circbuff_read edge case read after remove where start has changed
- [ ] ram
  - [ ] handle region overlap
- [ ] rtc
  - [ ] pretty much everything
- [ ] dir
  - [ ] pretty much everything
- [ ] file
  - [ ] pretty much everything
- [ ] Handle page faults
  - [ ] eg. add pages for user space stack
- [ ] Ring 3
  - [x] load code
  - [ ] add gdt
  - [ ] setup tss
  - [x] kernel interrupts
- [ ] Document code
  - [ ] cpu/idt.h
  - [ ] cpu/isr.h
  - [ ] cpu/mmu.h
  - [ ] cpu/ports.h
  - [ ] cpu/ram.h
  - [ ] drivers/timer.h
  - [ ] drivers/ata.h
  - [ ] drivers/keyboard.h
  - [ ] drivers/ramdisk.h
  - [ ] drivers/rtc.h
  - [ ] drivers/tar.h
  - [ ] drivers/vga.h
  - [x] libc/circbuff.h
  - [ ] libc/dir.h
  - [ ] libc/file.h
  - [ ] libc/memory.h
  - [ ] libc/stdio.h
  - [ ] libc/string.h
  - [ ] commands.h
  - [ ] debug.h
  - [ ] defs.h
  - [ ] disk.h
  - [ ] kernel.h
  - [ ] term.h
- [ ] Test code
  - [ ] cpu/idt.h
  - [ ] cpu/isr.h
  - [ ] cpu/mmu.h
  - [ ] cpu/ports.h
  - [ ] cpu/ram.h
  - [ ] drivers/timer.h
  - [ ] drivers/ata.h
  - [ ] drivers/keyboard.h
  - [ ] drivers/ramdisk.h
  - [ ] drivers/rtc.h
  - [ ] drivers/tar.h
  - [ ] drivers/vga.h
  - [x] libc/circbuff.h
  - [ ] libc/dir.h
  - [ ] libc/file.h
  - [x] libc/memory.h
  - [ ] libc/stdio.h
  - [x] libc/string.h
  - [ ] commands.h
  - [ ] debug.h
  - [ ] defs.h
  - [ ] disk.h
  - [ ] kernel.h
  - [ ] term.h
  - [x] sys_call.h
- [ ] Move drivers and other os level code out of kernel (only keep essentials)

## _Template Task_

Description of task and any relevant details / information.

### Tasks

- [ ] Sub task list

# Completed

## Physical Allocator

How to access region bitmask in paging mode?

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
