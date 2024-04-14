#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "term.h"
#include "std.h"

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
        term_puti(i, 10, false);
        term_putc('\n');
    }
    term_print("END\n\nTWO\n");
}
void kernel_main() {
    term_init();
    test1();
    test2();
    test3();
    test4();
}
