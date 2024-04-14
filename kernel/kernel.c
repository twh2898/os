#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "term.h"

void app();

void kernel_main() {
    term_init();

    int i = VGA_COLS * 2;
    for (char l = 'A'; l <= 'Z'; l++) {
        vga_put(i, l, VGA_WHITE_ON_BLACK);
        i++;
    }
    app();
}
