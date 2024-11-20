#ifndef KERNEL_H
#define KERNEL_H

#include "debug.h"

#ifndef SAFETY
/* Safety Modes
1 - Memory Safety
2 - Ptr Arg Safety
*/
#define SAFETY 1
// #define SAFETY 2
#endif

#define KERNEL_PANIC(MSG) kernel_panic((MSG), __FILE__, __LINE__)

_Noreturn void kernel_panic(const char * msg, const char * file, unsigned int line);

#define kassert(CHECK)                            \
    if (!(CHECK)) {                               \
        KERNEL_PANIC("assertion failed " #CHECK); \
    }
#define kassert_msg(CHECK, MSG)                             \
    if (!(CHECK)) {                                         \
        KERNEL_PANIC("assertion failed " #CHECK " : " MSG); \
    }

enum PHYS_PAGE_ADDR {
    PHYS_PAGE_ADDR_NULL = 0,
    PHYS_PAGE_ADDR_PAGE_DIR = 0x1000,
    PHYS_PAGE_ADDR_PAGE_TABLE = 0x2000,
    PHYS_PAGE_ADDR_KERNEL = 0x7e00,
};

enum VIRT_PAGE_ADDR {
    VIRT_PAGE_ADDR_NULL = 0,
    VIRT_PAGE_ADDR_PAGE_DIR = 0x1000,
    VIRT_PAGE_ADDR_RAM_TABLE = 0x2000,
    VIRT_PAGE_ADDR_KERNEL = 0x7000, // TODO: ??? Why is this not 0x7e00?
    VIRT_PAGE_ADDR_PAGE_TABLE = 0xffc00000,
};

#endif // KERNEL_H
