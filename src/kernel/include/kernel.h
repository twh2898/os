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

#endif // KERNEL_H
