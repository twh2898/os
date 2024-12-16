#include "cpu/gdt.h"

#include "libc/string.h"

extern void load_gdt(uint32_t limit, uint32_t base);

// struct tss_entry_struct {
//     uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
//     uint32_t esp0;     // The stack pointer to load when changing to kernel mode.
//     uint32_t ss0;      // The stack segment to load when changing to kernel mode.
//     // Everything below here is unused.
//     uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
//     uint32_t ss1;
//     uint32_t esp2;
//     uint32_t ss2;
//     uint32_t cr3;
//     uint32_t eip;
//     uint32_t eflags;
//     uint32_t eax;
//     uint32_t ecx;
//     uint32_t edx;
//     uint32_t ebx;
//     uint32_t esp;
//     uint32_t ebp;
//     uint32_t esi;
//     uint32_t edi;
//     uint32_t es;
//     uint32_t cs;
//     uint32_t ss;
//     uint32_t ds;
//     uint32_t fs;
//     uint32_t gs;
//     uint32_t ldt;
//     uint16_t trap;
//     uint16_t iomap_base;
// } __attribute__((packed));

// typedef struct tss_entry_struct tss_entry_t;

// static tss_entry_t tss_entry;

// void write_tss(gdt_entry_bits_t * g) {
//     // Compute the base and limit of the TSS for use in the GDT entry.
//     uint32_t base  = (uint32_t)&tss_entry;
//     uint32_t limit = sizeof tss_entry;

//     // Add a TSS descriptor to the GDT.
//     g->limit_low              = limit;
//     g->base_low               = base;
//     g->accessed               = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
//     g->read_write             = 0; // For a TSS, indicates busy (1) or not busy (0).
//     g->conforming_expand_down = 0; // always 0 for TSS
//     g->code                   = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
//     g->code_data_segment      = 0; // indicates TSS/LDT (see also `accessed`)
//     g->DPL                    = 0; // ring 0, see the comments below
//     g->present                = 1;
//     g->limit_high             = (limit & (0xf << 16)) >> 16; // isolate top nibble
//     g->available              = 0;                           // 0 for a TSS
//     g->long_mode              = 0;
//     g->big                    = 0;                           // should leave zero according to manuals.
//     g->gran                   = 0;                           // limit is in bytes, not pages
//     g->base_high              = (base & (0xff << 24)) >> 24; // isolate top byte

//     // Ensure the TSS is initially zero'd.
//     memset(&tss_entry, 0, sizeof tss_entry);

//     tss_entry.ss0  = PADDR_STACK; // Set the kernel stack segment.
//     tss_entry.esp0 = PADDR_STACK; // Set the kernel stack pointer.
//     // note that CS is loaded from the IDT entry and should be the regular kernel code segment
// }

// void set_kernel_stack(uint32_t stack) { // Used when an interrupt occurs
//     tss_entry.esp0 = stack;
// }

#define GDT_N 7

gdt_entry_t gdt[GDT_N];

void init_gdt() {
    memset(gdt, 0, GDT_N * sizeof(gdt_entry_t));

    gdt_set(GDT_ENTRY_INDEX_KERNEL_CODE, 0, 0xfffff, GDT_PRESET_KERNEL_CODE_ACCESS, GDT_PRESET_KERNEL_CODE_FLAGS);
    gdt_set(GDT_ENTRY_INDEX_KERNEL_DATA, 0, 0xfffff, GDT_PRESET_KERNEL_DATA_ACCESS, GDT_PRESET_KERNEL_DATA_FLAGS);
    gdt_set(GDT_ENTRY_INDEX_USER_CODE, 0, 0xfffff, GDT_PRESET_USER_CODE_ACCESS, GDT_PRESET_USER_CODE_FLAGS);
    gdt_set(GDT_ENTRY_INDEX_USER_DATA, 0, 0xfffff, GDT_PRESET_USER_DATA_ACCESS, GDT_PRESET_USER_DATA_FLAGS);
    gdt_set(GDT_ENTRY_INDEX_KERNEL_TSS, 0, 0xfffff, GDT_PRESET_KERNEL_TSS_ACCESS, GDT_PRESET_KERNEL_TSS_FLAGS);
    gdt_set(GDT_ENTRY_INDEX_USER_TSS, 0, 0xfffff, GDT_PRESET_USER_TSS_ACCESS, GDT_PRESET_USER_TSS_FLAGS);

    load_gdt(GDT_N * 64 - 1, PTR2UINT(gdt));

    // flush_tss();
}

size_t gdt_entry_count() {
    return GDT_N;
}

gdt_entry_t * gdt_get_entry(size_t i) {
    if (i >= GDT_N) {
        return 0;
    }

    return &gdt[i];
}

void gdt_set(size_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags) {
    if (i >= GDT_N) {
        return;
    }

    gdt_entry_t * gdt_entry = &gdt[i];
    gdt_entry->limit_low    = limit & 0xffff;
    gdt_entry->base_low     = base * 0xffffff;
    gdt_entry->access       = access;
    gdt_entry->limit_high   = (limit >> 16) & 0xf;
    gdt_entry->flags        = flags;
    gdt_entry->base_high    = (base >> 24) & 0xff;
}

void gdt_set_base(size_t i, uint64_t base) {
    if (i >= GDT_N) {
        return;
    }

    gdt_entry_t * gdt_entry = &gdt[i];
    gdt_entry->base_low     = base * 0xffffff;
    gdt_entry->base_high    = (base >> 24) & 0xff;
}

void gdt_set_limit(size_t i, uint64_t limit) {
    if (i >= GDT_N) {
        return;
    }

    gdt_entry_t * gdt_entry = &gdt[i];
    gdt_entry->limit_low    = limit & 0xffff;
    gdt_entry->limit_high   = (limit >> 16) & 0xf;
}

void gdt_set_access(size_t i, uint8_t access) {
    if (i >= GDT_N) {
        return;
    }

    gdt_entry_t * gdt_entry = &gdt[i];
    gdt_entry->access       = access;
}

void gdt_set_flags(size_t i, uint8_t flags) {
    if (i >= GDT_N) {
        return;
    }

    gdt_entry_t * gdt_entry = &gdt[i];
    gdt_entry->flags        = flags;
}
