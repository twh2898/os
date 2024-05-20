#ifndef KERNEL_H
#define KERNEL_H

#include "debug.h"

#define KERNEL_PANIC(MSG) kernel_panic((MSG), __FILE__, __LINE__)

void kernel_panic(const char * msg, const char * file, unsigned int line);

#endif // KERNEL_H
