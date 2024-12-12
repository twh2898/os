#include "cpu/gdt.h"

#include "cpu/boot_params.h"
#include "libc/string.h"

struct gdt_entry_bits {
    unsigned int limit_low : 16;
    unsigned int base_low : 24;
    unsigned int accessed : 1;
    unsigned int read_write : 1; // readable for code, writable for data
    unsigned int conforming_expand_down : 1; // conforming for code, expand down for data
    unsigned int code : 1; // 1 for code, 0 for data
    unsigned int code_data_segment : 1; // should be 1 for everything but TSS and LDT
    unsigned int DPL : 2; // privilege level
    unsigned int present : 1;
    unsigned int limit_high : 4;
    unsigned int available : 1; // only used in software; has no effect on hardware
    unsigned int long_mode : 1;
    unsigned int big : 1; // 32-bit opcodes for code, uint32_t stack for data
    unsigned int gran : 1; // 1 to use 4k page addressing, 0 for byte addressing
    unsigned int base_high : 8;
} __attribute__((packed));

typedef struct gdt_entry_bits gdt_entry_bits_t;

struct gdt_descriptor {

} __attribute__((packed));

typedef struct gdt_descriptor gdt_descriptor_t;

extern void load_gdt(uint32_t limit, uint32_t base);

// null + kernel code/data + user code/data + tss
static gdt_entry_bits_t gdt[6];

static gdt_descriptor_t gdt_descriptor;

struct tss_entry_struct {
    uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
    uint32_t esp0; // The stack pointer to load when changing to kernel mode.
    uint32_t ss0; // The stack segment to load when changing to kernel mode.
    // Everything below here is unused.
    uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __packed;

typedef struct tss_entry_struct tss_entry_t;

static tss_entry_t tss_entry;

void write_tss(gdt_entry_bits_t * g) {
    // Compute the base and limit of the TSS for use in the GDT entry.
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof tss_entry;

    // Add a TSS descriptor to the GDT.
    g->limit_low = limit;
    g->base_low = base;
    g->accessed = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
    g->read_write = 0; // For a TSS, indicates busy (1) or not busy (0).
    g->conforming_expand_down = 0; // always 0 for TSS
    g->code = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
    g->code_data_segment = 0; // indicates TSS/LDT (see also `accessed`)
    g->DPL = 0; // ring 0, see the comments below
    g->present = 1;
    g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
    g->available = 0; // 0 for a TSS
    g->long_mode = 0;
    g->big = 0; // should leave zero according to manuals.
    g->gran = 0; // limit is in bytes, not pages
    g->base_high = (base & (0xff << 24)) >> 24; // isolate top byte

    // Ensure the TSS is initially zero'd.
    memset(&tss_entry, 0, sizeof tss_entry);

    tss_entry.ss0 = PADDR_STACK; // Set the kernel stack segment.
    tss_entry.esp0 = PADDR_STACK; // Set the kernel stack pointer.
    // note that CS is loaded from the IDT entry and should be the regular kernel code segment
}

void set_kernel_stack(uint32_t stack) { // Used when an interrupt occurs
    tss_entry.esp0 = stack;
}

extern void flush_tss();

void gdt_init() {
    boot_params_t * bparams = get_boot_params();

    gdt_entry_bits_t * source_gdt = UINT2PTR(bparams->gdt_addr);

    for (size_t i = 0; i < 5; i++) {
        gdt[i] = source_gdt[i];
    }

    gdt_entry_bits_t * ring3_code = &gdt[3];
    gdt_entry_bits_t * ring3_data = &gdt[4];

    ring3_code->limit_low = 0xFFFF;
    ring3_code->base_low = 0;
    ring3_code->accessed = 0;
    ring3_code->read_write = 1; // since this is a code segment, specifies that the segment is readable
    ring3_code->conforming_expand_down = 0; // does not matter for ring 3 as no lower privilege level exists
    ring3_code->code = 1;
    ring3_code->code_data_segment = 1;
    ring3_code->DPL = 3; // ring 3
    ring3_code->present = 1;
    ring3_code->limit_high = 0xF;
    ring3_code->available = 1;
    ring3_code->long_mode = 0;
    ring3_code->big = 1; // it's 32 bits
    ring3_code->gran = 1; // 4KB page addressing
    ring3_code->base_high = 0;

    *ring3_data = *ring3_code; // contents are similar so save time by copying
    ring3_data->code = 0; // not code but data

    write_tss(&gdt[5]);

    load_gdt(6 * 64 - 1, PTR2UINT(gdt));

    flush_tss();
}
