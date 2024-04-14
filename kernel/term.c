#include "term.h"

#include "../drivers/ports.h"
#include "../drivers/vga.h"

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#define MAX_INDEX (VGA_ROWS * VGA_COLS)

static int index;
static char color;

static void update_cursor();
static void shift_lines();

void term_init() {
    index = 0;
    color = VGA_WHITE_ON_BLACK;
    vga_clear();
    update_cursor();
}

void term_putc(char c) {
    if (index >= MAX_INDEX) {
        shift_lines();
        index = vga_index(VGA_ROWS-1, 0);
    }

    if (c == '\n') {
        int row = vga_row(index);
        index = vga_index(row + 1, 0);
    }
    else {
        vga_put(index++, c, VGA_WHITE_ON_BLACK);
    }
    update_cursor();
}

void term_print(const char * str) {
    while (*str != 0) {
        term_putc(*str++);
    }
}

static void update_cursor() {
    if (index < 0 || index >= MAX_INDEX)
        return;

    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index & 0xff));
}

static void shift_lines() {
    char * screen = (char *) VGA_ADDRES;
    for (int row = 1; row < VGA_ROWS; row++) {
        for (int col = 1; col < VGA_COLS; col++) {
            int index = vga_index(row, col);
            int prev = index - (VGA_COLS * 2);
            screen[prev] = screen[index];
            screen[prev+1] = screen[index+1];
        }
    }
    for (int col = 1; col < VGA_COLS; col++) {
        int index = vga_index(VGA_ROWS - 1, col);
        vga_put(index, ' ', VGA_WHITE_ON_BLACK);
    }
}
