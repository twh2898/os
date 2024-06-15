#include "kernel.h"

#include "commands.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "defs.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/page.h"
#include "drivers/ram.h"
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
    int len = kprintf(
        "Like the percent sign %%, \na signed int %d, a signed int with width formatting %4d, \nleading zeros %04d, left align %-4d\n",
        10,
        10,
        10,
        10);
    len += kprintf(
        "How about negative numbers: signed %d and unsigned %u\n", -10, -10);
    len += kprintf(
        "Now for non decimal 0x%04x and 0x%04X or octal %o\n", 1234, 1234, 1234);
    len += kprintf("There's booleans to %b and chars like %c and strings like %s\n",
                   true,
                   'c',
                   "this");
    int store = 0;
    len += kprintf("The last part is pointers %8p\n", &store);

    void * data = kmalloc(10);

    kprintf("\nMalloc memory got pointer %p\n", data);
    kprintf("Float number %f or shorter %3f or digits %.4f or lead %.04f\n",
            3.14,
            31.45,
            3.14,
            3.14);
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

static mmu_page_dir_t * pdir;
static mmu_page_table_t * ptable;

static void identity_map(mmu_page_table_t * table, size_t n_pages) {
    if (n_pages > 1024)
        n_pages = 1024;

    for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        if (i > 10) {
            mmu_table_set_addr(table, i, 0);
            mmu_table_set_flags(table, i, 0);
            continue;
        }

        mmu_table_set_addr(table, i, i << 12);
        enum MMU_PAGE_TABLE_FLAG flags = 0;
        if (i)
            flags = MMU_TABLE_RW;
        mmu_table_set_flags(table, i, flags);
    }
}

static void map_virt_page_dir(mmu_page_dir_t * dir) {
    mmu_page_table_t * lastTable = page_alloc();
    mmu_table_create(lastTable);
    dir->entries[PAGE_DIR_SIZE - 1] =
        (PTR2UINT(lastTable) * MASK_ADDR) |   MMU_TABLE_RW;

    lastTable->entries[0] = (PTR2UINT(dir) & MASK_ADDR) | MMU_TABLE_RW;

    // TODO: create more entries for each page table as they are requested from malloc
}

static void enter_paging() {
    pdir = mmu_dir_create((void *)0x1000);
    ptable = mmu_table_create((void *)0x2000);

    mmu_dir_set_table(pdir, 0, ptable);
    mmu_dir_set_flags(pdir, 0, MMU_DIR_RW);

    identity_map(ptable, 10);
    map_virt_page_dir(pdir);

    mmu_enable_paging(pdir);
}

void kernel_main() {
    vga_clear();

    isr_install();
    irq_install();

    vga_print("Welcome to kernel v..\n");

    init_ram();
    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    init_pages();
    enter_paging();
    // trigger_page_fault();
    // kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    init_malloc(pdir, 10);

    // init_ata();

    // term_init();
    // commands_init();

    // term_command_add("demo", demo);
    // term_command_add("test", test_cmd);

    // ramdisk_create(4096);

    // term_run();
}
