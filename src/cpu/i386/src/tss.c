#include "cpu/tss.h"

#include "cpu/gdt.h"
#include "libc/string.h"

#define TSS_N 2
tss_entry_t tss_stack[TSS_N];

extern void flush_tss();
extern void jump_usermode(void * fn);

void init_tss() {
    memset(tss_stack, 0, sizeof(tss_stack));

    gdt_set_base(GDT_ENTRY_INDEX_KERNEL_TSS, PTR2UINT(&tss_stack[0]));
    gdt_set_base(GDT_ENTRY_INDEX_USER_TSS, PTR2UINT(&tss_stack[1]));

    flush_tss();
}

tss_entry_t * tss_get_entry(size_t i) {
    if (i >= TSS_N) {
        return 0;
    }

    return &tss_stack[i];
}
