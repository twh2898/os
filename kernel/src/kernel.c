#include "kernel.h"

#include "commands.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/page.h"
#include "drivers/ram.h"
#include "drivers/ramdisk.h"
#include "drivers/vga.h"
#include "libc/mem.h"
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

    void * data = malloc(10);

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

static void enter_paging() {
    pdir = mmu_dir_create();
    ptable = mmu_table_create();

    mmu_dir_set_table(pdir, 0, ptable);
    mmu_dir_set_flags(
        pdir, 0, MMU_PAGE_DIR_FLAGS_PRESENT | MMU_PAGE_DIR_FLAGS_READ_WRITE);

    for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        mmu_table_set_addr(ptable, i, i << 12);
        if (i == 0)
        mmu_table_set_flags(
            ptable, i, 0);
        else
        mmu_table_set_flags(
            ptable, i, MMU_PAGE_TABLE_FLAGS_PRESENT | MMU_PAGE_TABLE_FLAGS_READ_WRITE);
    }

    mmu_enable_paging(pdir);
}

void kernel_main() {
    vga_clear();

    isr_install();
    irq_install();

    vga_print("Welcome to kernel v..\n");

    init_ram();
    kprintf("Paging enabled: %b\n", mmu_paging_enabled());
    init_pages();
    enter_paging();
    // trigger_page_fault();
    // init_malloc();
    kprintf("Paging enabled: %b\n", mmu_paging_enabled());

    // init_ata();

    // term_init();
    // commands_init();

    // term_command_add("demo", demo);
    // term_command_add("test", test_cmd);

    // ramdisk_create(4096);

    // term_run();
}
