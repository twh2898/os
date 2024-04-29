#include "commands.h"
#include "cpu/isr.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"
#include "test.h"

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
    int len = printf(
        "Like the percent sign %%, \na signed int %d, a signed int with width formatting %4d, \nleading zeros %04d, left align %-4d\n",
        10,
        10,
        10,
        10);
    len += printf(
        "How about negative numbers: signed %d and unsigned %u\n", -10, -10);
    len += printf(
        "Now for non decimal 0x%04x and 0x%04X or octal %o\n", 1234, 1234, 1234);
    len += printf("There's booleans to %b and chars like %c and strings like %s\n",
                  true,
                  'c',
                  "this");
    int store = 0;
    len += printf("The last part is pointers %8p\n", &store);

    void * data = malloc(10);

    printf("\nMalloc memory got pointer %p\n", data);
    printf("Float number %f or shorter %3f or digits %.4f or lead %.04f\n",
           3.14,
           31.45,
           3.14,
           3.14);
    printf("%f\n", 12345678.0);
}

void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        putc(c);
    }
}

static int test_cmd(size_t argc, char ** argv) {
    return run_tests();
}

void console() {
    term_init();
    commands_init();
    term_command_add("demo", demo);
    term_command_add("test", test_cmd);
    test_cmd(0, 0);
}

void kernel_main() {
    vga_clear();
    isr_install();
    irq_install();

    // cursor();
    // test_interrupt();
    // demo(0, 0);

    console();
}
