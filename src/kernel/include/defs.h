#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MASK_ADDR 0xfffff000
#define MASK_FLAGS 0xfff

#define PAGE_SIZE 4096
#define PAGE_ALIGNED_DOWN(PTR) ((PTR) & MASK_ADDR)
#define PAGE_ALIGNED_UP(PTR) ((PAGE_ALIGNED_DOWN(PTR)) + PAGE_SIZE)
#define PAGE_ALIGNED(PTR) (((PTR) & MASK_FLAGS) ? PAGE_ALIGNED_UP(PTR) : (PTR))

#define PTR2UINT(PTR) ((uint32_t)(PTR))
#define UINT2PTR(UINT) ((void *)(UINT))

#endif // DEFS_H
