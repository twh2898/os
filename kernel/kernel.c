#include "../drivers/ports.h"
#include "../drivers/vga.h"
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
    term_print("\nHello\nWorld");

    term_print("\n\n\n\n\n\n\n\n\n\n");
    term_print("\n\n\n\n\n\n\n\n\n\n");
    term_print("1\n2\n3\n4\n5\n6\n7\n8");
}
void kernel_main() {
    term_init();
    test1();
    test2();
    // test3();
}
