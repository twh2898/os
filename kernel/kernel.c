#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
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
    len += printf("The last part is pointers 0x%8p\n", &store);
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

void kernel_main() {
    term_init();
    isr_install();

    // test1();
    // test2();
    // test3();
    // test4();
    // demo();
    // cursor();
    // test_interrupt();

    asm volatile("sti");

    init_timer(50);

    init_keyboard();
}
