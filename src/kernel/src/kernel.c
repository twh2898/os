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

static void map_virt_page_dir(mmu_page_dir_t * dir) {
    size_t ram_table_count;
    mmu_page_table_t * firstTable = init_ram(UINT2PTR(PADDR_RAM_TABLE), &ram_table_count);
    kprintf("Page table created at %p\n", firstTable);
    mmu_table_create(firstTable);
    mmu_dir_set(dir, 0, firstTable, MMU_DIR_RW);

    // null page 0
    mmu_table_set(firstTable, 0, 0, 0);

    // Page Directory
    mmu_table_set(firstTable, 1, PADDR_PAGE_DIR, MMU_TABLE_RW);

    // Ram region table
    mmu_table_set(firstTable, 2, PADDR_RAM_TABLE, MMU_TABLE_RW);

    // stack (4)
    for (size_t i = 0; i < 4; i++) {
        mmu_table_set(firstTable, 3 + i, PADDR_STACK + (i << 12), MMU_TABLE_RW);
    }

    // kernel (0x98)
    for (size_t i = 0; i < 0x98; i++) {
        mmu_table_set(firstTable, 3 + i, PADDR_KERNEL + (i << 12), MMU_TABLE_RW);
    }

    // ram region bitmasks
    for (size_t i = 0; i < 512; i++) {
        uint32_t addr = get_bitmask_addr(i);
        if (addr)
            mmu_table_set(firstTable, 0xa1 + i, 0xa1000 + (i << 12), MMU_TABLE_RW);
        else
            mmu_table_set(firstTable, 0xa1 + i, 0, 0);
    }

    /* SKIP FREE MEMORY */

    // create page table for the last entry of the page directory
    mmu_page_table_t * lastTable = firstTable + PAGE_SIZE;
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

void kernel_main() {
    vga_clear();

    isr_install();
    irq_install();

    vga_print("Welcome to kernel v..\n");

    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    mmu_page_dir_t * pdir = enter_paging();
    // trigger_page_fault();
    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    init_malloc(pdir, 0x9f + 512);

    // init_ata();

    // term_init();
    // commands_init();

    // term_command_add("demo", demo);
    // term_command_add("test", test_cmd);

    // ramdisk_create(4096);

    // term_run();
}
