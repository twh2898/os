#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MASK_ADDR  0xfffff000
#define MASK_FLAGS 0xfff

#define PAGE_SIZE              4096
#define PAGE_ALIGNED_DOWN(PTR) ((PTR) & MASK_ADDR)
#define PAGE_ALIGNED_UP(PTR)   ((PAGE_ALIGNED_DOWN(PTR)) + PAGE_SIZE)
#define PAGE_ALIGNED(PTR)      (((PTR) & MASK_FLAGS) ? PAGE_ALIGNED_UP(PTR) : (PTR))

#define PAGE2ADDR(PAGE) ((PAGE) << 12)
#define ADDR2PAGE(ADDR) ((ADDR) >> 12)

#define PTR2UINT(PTR)   ((uint32_t)(PTR))
#define UINT2PTR(UINT)  ((void *)(UINT))
#define LUINT2PTR(UINT) UINT2PTR((uint32_t)(UINT))

#define PADDR_BOOT_PARAMS 0x500
#define PADDR_PAGE_DIR    0x1000
#define PADDR_RAM_TABLE   0x2000
#define PADDR_STACK       0x3000
#define PADDR_GDT         0x7c00
#define PADDR_KERNEL      0x7e00
#define PADDR_VGA         0xb8000

// Identity mapped
#define VADDR_NULL      0x0
#define VADDR_PAGE_DIR  PADDR_PAGE_DIR
#define VADDR_RAM_TABLE PADDR_RAM_TABLE
#define VADDR_STACK     PADDR_STACK
#define VADDR_KERNEL    PADDR_KERNEL
#define VADDR_VGA       PADDR_VGA

// Physical address allocated at runtime
#define VADDR_RAM_BITMASKS    (VADDR_VGA + PAGE_SIZE)
#define VADDR_FREE_MEM_KERNEL (VADDR_RAM_BITMASKS + PAGE_SIZE * 0x200)
#define VADDR_FREE_MEM_USER   0x400000

#define VADDR_FIRST_PAGE_TABLE  0xffc00000
#define VADDR_SECOND_PAGE_TABLE 0xffc01000
#define VADDR_LAST_PAGE_TABLE   0xfffff000

#endif // DEFS_H
