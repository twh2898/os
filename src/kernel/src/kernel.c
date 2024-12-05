#include "kernel.h"

#include "commands.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/ram.h"
#include "cpu/timer.h"
#include "defs.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/ramdisk.h"
#include "drivers/vga.h"
#include "libc/memory.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"
#include "test.h"

_Noreturn void kernel_panic(const char * msg, const char * file, unsigned int line) {
    vga_color(VGA_FG_WHITE | VGA_BG_RED);
    vga_print("[KERNEL PANIC]");
    if (file)
        kprintf("[%s]:%u", file, line);
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
    kprintf("Lets demo some cool features of kprintf\n");
    int len = kprintf("Like the percent sign %%, \na signed int %d, a signed int with width formatting %4d, \nleading zeros %04d, left align %-4d\n", 10, 10, 10, 10);
    len += kprintf("How about negative numbers: signed %d and unsigned %u\n", -10, -10);
    len += kprintf("Now for non decimal 0x%04x and 0x%04X or octal %o\n", 1234, 1234, 1234);
    len += kprintf("There's booleans to %b and chars like %c and strings like %s\n", true, 'c', "this");
    int store = 0;
    len += kprintf("The last part is pointers %8p\n", &store);

    void * data = kmalloc(10);

    kprintf("\nMalloc memory got pointer %p\n", data);
    kprintf("Float number %f or shorter %3f or digits %.4f or lead %.04f\n", 3.14, 31.45, 3.14, 3.14);
    kprintf("%f\n", 12345678.0);
}

void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        kputc(c);
    }
}

static int test_cmd(size_t argc, char ** argv) {
    return run_tests();
}

static void trigger_page_fault() {
    uint8_t * v = (uint8_t *)0;
    *v = 0;
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
    size_t ram_table_count;
    mmu_page_table_t * firstTable = init_ram(UINT2PTR(PADDR_RAM_TABLE), &ram_table_count);
    // kprintf("First table is at 0x%x\n", PTR2UINT(firstTable));
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
    // kprintf("Last table created at %p\n", lastTable);
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

    kprintf("Ram page alloc gave 0x%x and 0x%X\n", PTR2UINT(a), PTR2UINT(b));

    void * c = kmalloc(1);
    void * d = kmalloc(1);

    kprintf("Malloc gave vaddr %p and %p\n", c, d);

    void * e = ram_page_alloc();

    kprintf("Ram page alloc gave %p\n", e);
}

static void yea_callback(registers_t regs) {
    kprintf("Yea Baybee!");
}

void kernel_main() {
    vga_clear();

    isr_install();
    irq_install();

    register_interrupt_handler(IRQ16, yea_callback);

    vga_print("Welcome to kernel v..\n");

    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    mmu_page_dir_t * pdir = enter_paging();
    // trigger_page_fault();
    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    init_malloc(pdir, VADDR_FREE_MEM_KERNEL >> 12);

    // check_malloc();

    term_init();
    commands_init();

    term_command_add("demo", demo);
    term_command_add("test", test_cmd);

    ramdisk_create(4096);

    // run_tests();

    // uint32_t r = system_call(4, 1);
    // kprintf("System call got 0x%X\n", r);

    term_run();
}
