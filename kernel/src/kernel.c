#include "kernel.h"

#include "commands.h"
#include "cpu/isr.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/ram.h"
#include "drivers/vga.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"
#include "test.h"

void kernel_panic(const char * msg, const char * file, unsigned int line) {
    vga_color(VGA_FG_WHITE | VGA_BG_RED);
    vga_print("[KERNEL PANIC]");
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

void kernel_main() {
    vga_clear();
    vga_print("Welcome to kernel v..\n");

    isr_install();
    irq_install();

    init_ram();
    init_malloc();
    init_disk();

    term_init();
    commands_init();

    term_command_add("demo", demo);
    term_command_add("test", test_cmd);

    term_run();
}
