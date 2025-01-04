#ifndef ADDR_H
#define ADDR_H

#include <stdint.h>

typedef uint32_t p_addr_t;
typedef uint32_t v_addr_t;

typedef void * p_ptr_t;
typedef void * v_ptr_t;

#define PTR2ADDR(PTR)  ((uint32_t)(PTR))
#define ADDR2PTR(ADDR) ((void *)(ADDR))

// Physical addresses (mostly for boot and kernel)
#define PADDR_BOOT_PARAMS 0x500
#define PADDR_PAGE_DIR    0x1000
#define PADDR_RAM_TABLE   0x2000
#define PADDR_STACK       0x3000
#define PADDR_GDT         0x7c00
#define PADDR_KERNEL      0x7e00
#define PADDR_VGA         0xb8000

// Identity mapped
#define VADDR_NULL           0x0
#define VADDR_PAGE_DIR       PADDR_PAGE_DIR
#define VADDR_RAM_TABLE      PADDR_RAM_TABLE
#define VADDR_STACK          PADDR_STACK
#define VADDR_KERNEL         PADDR_KERNEL
#define VADDR_TMP_PAGE       0x9f000
#define VADDR_TMP_PAGE_COUNT 0x19
#define VADDR_VGA            PADDR_VGA

// Physical address allocated at runtime
#define VADDR_RAM_BITMASKS    (VADDR_VGA + PAGE_SIZE)
#define VADDR_FREE_MEM_KERNEL (VADDR_RAM_BITMASKS + PAGE_SIZE * 0x200)
#define VADDR_ISR_STACK       0x3fffff
#define VADDR_FREE_MEM_USER   0x400000

#define VADDR_FIRST_PAGE_TABLE  0xffc00000
#define VADDR_SECOND_PAGE_TABLE 0xffc01000
#define VADDR_LAST_PAGE_TABLE   0xfffff000

#endif // ADDR_H
