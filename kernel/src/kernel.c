#include "cpu/isr.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"

void test1() {
    int i = VGA_COLS * 2;
    for (char l = 'A'; l <= 'Z'; l++) {
        vga_put(i, l, VGA_WHITE_ON_BLACK);
        i++;
    }
}

void test2() {
    for (char l = 'A'; l <= 'Z'; l++) {
        term_putc(l);
    }
}

void test3() {
    term_print("Hello\nWorld");
    term_cursor(1, 10);
    term_color(VGA_FG_CYAN | VGA_BG_DARK_GRAY);
    term_print("\nHello\nWorld");
}

void test4() {
    term_cursor(0, 0);
    term_color(VGA_WHITE_ON_BLACK);
    for (int i = 0; i < 24; i++) {
        term_print("FILL ");
        puti(i, 10, false);
        term_putc('\n');
    }
    term_print("END\n\nTWO\n");
}

void cursor() {
    term_cursor(3, 3);

    term_cursor_hide();
    term_cursor_show();
    term_cursor(term_cursor_row(), term_cursor_col());
}

void test_interrupt() {
    vga_clear();
    /* Test the interrupts */
    asm volatile("int $2");
    asm volatile("int $3");
}

void demo() {
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
}

void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        putc(c);
    }
}

void console() {
    keyboard_set_cb(&key_cb);
    printf("\n> ");
}

void kernel_main() {
    term_init();
    isr_install();
    irq_install();

    // test1();
    // test2();
    // test3();
    // test4();
    // cursor();
    // test_interrupt();

    demo();
    console();
}
