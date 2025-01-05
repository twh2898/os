#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "addr.h"

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

#define VIRTUAL_TABLE(I) ((mmu_table_t *)(VADDR_FIRST_PAGE_TABLE + ((I) << 12)))

#endif // DEFS_H
