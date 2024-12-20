#include "kernel.h"

#include "commands.h"
#include "cpu/gdt.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/ram.h"
#include "cpu/timer.h"
#include "cpu/tss.h"
#include "defs.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/ramdisk.h"
#include "drivers/rtc.h"
#include "drivers/vga.h"
#include "interrupts.h"
#include "libc/memory.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "libk/defs.h"
#include "term.h"

_Noreturn void kernel_panic(const char * msg, const char * file, unsigned int line) {
    vga_color(VGA_FG_WHITE | VGA_BG_RED);
    vga_print("[KERNEL PANIC]");
    if (file) {
        vga_putc('[');
        vga_print(file);
        vga_print("]:");
        vga_putu(line);
    }
    if (msg) {
        vga_putc(' ');
        vga_print(msg);
    }
    vga_cursor_hide();
    asm("cli");
    for (;;) {
        asm("hlt");
    }
}

void cursor() {
    vga_cursor(3, 3);

    vga_cursor_hide();
    vga_cursor_show();
    vga_cursor(vga_cursor_row(), vga_cursor_col());
}

void test_interrupt() {
    vga_clear();
    /* Test the interrupts */
    asm volatile("int $2");
    asm volatile("int $3");
}

int demo(size_t argc, char ** argv) {
    printf("Lets demo some cool features of printf\n");
    int len = printf("Like the percent sign %%, \na signed int %d, a signed int with width formatting %4d, \nleading zeros %04d, left align %-4d\n", 10, 10, 10, 10);
    len += printf("How about negative numbers: signed %d and unsigned %u\n", -10, -10);
    len += printf("Now for non decimal 0x%04x and 0x%04X or octal %o\n", 1234, 1234, 1234);
    len += printf("There's booleans to %b and chars like %c and strings like %s\n", true, 'c', "this");
    int store = 0;
    len += printf("The last part is pointers %8p\n", &store);

    void * data = kmalloc(10);

    printf("\nMalloc memory got pointer %p\n", data);
    printf("Float number %f or shorter %3f or digits %.4f or lead %.04f\n", 3.14, 31.45, 3.14, 3.14);
    printf("%f\n", 12345678.0);
}

void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        putc(c);
    }
}

static void trigger_page_fault() {
    uint8_t * v = (uint8_t *)0;
    *v          = 0;
}

static void irq_install() {
    /* Enable interruptions */
    asm volatile("sti");
    /* IRQ0: timer */
    init_timer(1000); // milliseconds
    /* IRQ1: keyboard */
    init_keyboard();
    /* IRQ14: ata disk */
    init_ata();
    /* IRQ8: real time clock */
    init_rtc(RTC_RATE_1024_HZ);
}

static void id_map(mmu_page_table_t * table, size_t start, size_t end) {
    if (end > 1023) {
        KERNEL_PANIC("End is past table limits");
        end = 1023;
    }
    while (start <= end) {
        mmu_table_set(table, start, start << 12, MMU_TABLE_RW);
        start++;
    }
}

static void map_virt_page_dir(mmu_page_dir_t * dir) {
    size_t             ram_table_count;
    mmu_page_table_t * firstTable = init_ram(UINT2PTR(PADDR_RAM_TABLE), &ram_table_count);
    // printf("First table is at 0x%x\n", PTR2UINT(firstTable));
    mmu_table_create(firstTable);
    mmu_dir_set(dir, 0, firstTable, MMU_DIR_RW);

    // null page 0
    mmu_table_set(firstTable, 0, 0, 0);

    // Kernel
    id_map(firstTable, 1, 0x9e);

    /* SKIP UNUSED MEMORY */

    // VGA
    mmu_table_set(firstTable, 0xb8, PADDR_VGA, MMU_TABLE_RW);

    // RAM region bitmasks
    for (size_t i = 0; i < ram_table_count; i++) {
        mmu_table_set(firstTable, ADDR2PAGE(VADDR_RAM_BITMASKS) + i, ram_bitmask_paddr(i), MMU_TABLE_RW);
    }

    /* SKIP FREE MEMORY */

    // create page table for the last entry of the page directory
    mmu_page_table_t * lastTable = firstTable + PAGE_SIZE;
    // printf("Last table created at %p\n", lastTable);
    mmu_table_create(lastTable);
    mmu_dir_set(dir, PAGE_DIR_SIZE - 1, lastTable, MMU_DIR_RW);

    // first entry of the last page is the memory map
    mmu_table_set(lastTable, 0, PTR2UINT(firstTable), MMU_TABLE_RW);

    // last entry of the last page table points to the block of page tables
    mmu_table_set(lastTable, PAGE_TABLE_SIZE - 1, PTR2UINT(lastTable), MMU_TABLE_RW);
}

static mmu_page_dir_t * enter_paging() {
    mmu_page_dir_t * pdir = mmu_dir_create((void *)PADDR_PAGE_DIR);
    map_virt_page_dir(pdir);
    mmu_enable_paging(pdir);
    return pdir;
}

static void check_malloc() {
    void * a = ram_page_alloc();
    void * b = ram_page_alloc();

    printf("Ram page alloc gave 0x%x and 0x%X\n", PTR2UINT(a), PTR2UINT(b));

    void * c = kmalloc(1);
    void * d = kmalloc(1);

    printf("Malloc gave vaddr %p and %p\n", c, d);

    void * e = ram_page_alloc();

    printf("Ram page alloc gave %p\n", e);
}

static uint32_t int_mem_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_MEM_MALLOC: {
            size_t size = regs->ebx;
            res         = PTR2UINT(kmalloc(size));
        } break;

        case SYS_INT_MEM_REALLOC: {
            void * ptr  = UINT2PTR(regs->ebx);
            size_t size = regs->ecx;
            // res = PTR2UINT(krealloc(ptr, size));
        } break;

        case SYS_INT_MEM_FREE: {
            void * ptr = UINT2PTR(regs->ebx);
            kfree(ptr);
        } break;
    }

    return res;
}

static uint32_t int_proc_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_PROC_EXIT: {
            uint8_t code = regs->ebx;
            KERNEL_PANIC("Exit program!");
        } break;

        case SYS_INT_PROC_EXIT_ERROR: {
            uint8_t      code = regs->ebx;
            const char * msg  = UINT2PTR(regs->ecx);
            vga_print(msg);
            KERNEL_PANIC("Exit program!");
        } break;
    }

    return 0;
}

static uint32_t int_tmp_stdio_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_STDIO_PUTC: {
            char c = regs->ebx;
            res    = vga_putc(c);
        } break;

        case SYS_INT_STDIO_PUTS: {
            char * str = UINT2PTR(regs->ebx);
            res        = vga_print(str);
        } break;
    }

    return 0;
}

void kernel_main() {
    init_vga(UINT2PTR(PADDR_VGA));
    vga_clear();

    isr_install();
    irq_install();

    init_system_interrupts(IRQ16);
    system_interrupt_register(SYS_INT_FAMILY_MEM, int_mem_cb);
    system_interrupt_register(SYS_INT_FAMILY_PROC, int_proc_cb);
    system_interrupt_register(SYS_INT_FAMILY_STDIO, int_tmp_stdio_cb);

    vga_print("Welcome to kernel v..\n");

    // printf("Paging enabled: %b\n", mmu_paging_enabled());
    mmu_page_dir_t * pdir = enter_paging();
    // trigger_page_fault();
    // printf("Paging enabled: %b\n", mmu_paging_enabled());
    init_malloc(pdir, VADDR_FREE_MEM_KERNEL >> 12);

    init_gdt();
    // init_tss();

    // check_malloc();

    term_init();
    commands_init();

    term_command_add("demo", demo);

    ramdisk_create(4096);

    // uint32_t r = system_call(4, 1);
    // printf("System call got 0x%X\n", r);

    // sys_call_1();
    // sys_call_1();
    // sys_call_1();

    // jump_usermode(term_run);

    term_run();
}
