#include "term.h"

void app() {

    // test 2
    for (char l = 'A'; l <= 'Z'; l++) {
        term_putc(l);
    }

    // test 3

    term_print("Hello\nWorld");
    term_print("\nHello\nWorld");

    //term_print("\n\n\n\n\n\n\n\n\n\n");
    //term_print("\n\n\n\n\n\n\n\n\n\n");
    //term_print("1\n2\n3\n4\n5\n6\n7\n8");
}
