#ifndef GDT_H
#define GDT_H

#include "defs.h"

void gdt_init();

extern void jump_usermode(void * fn);

#endif // GDT_H
