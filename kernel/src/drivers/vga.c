#include "drivers/vga.h"

void vga_clear() {
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            int index = vga_index(row, col);
            vga_put(index, ' ', VGA_WHITE_ON_BLACK);
        }
    }
}

void vga_put(int index, char c, unsigned char attr) {
    char * screen = (char *)VGA_ADDRESS;
    index *= 2;
    screen[index] = c;
    screen[index + 1] = attr;
}

int vga_row(int index) {
    return index / VGA_COLS;
}

int vga_col(int index) {
    return index % VGA_COLS;
}

int vga_index(int row, int col) {
    return (row * VGA_COLS) + col;
}
