#include "drivers/vga.h"

#include "cpu/ports.h"
#include "libc/string.h"

#define VGA_ADDRESS 0xb8000
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#define MAX_INDEX (VGA_ROWS * VGA_COLS)

static int index = 0;
static char color = RESET;
static char * screen = (char *)VGA_ADDRESS;

static void update_cursor();
static void shift_lines();

/*
 * DIRECT ACCESS
 */

void vga_clear() {
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            index = vga_index(row, col);
            vga_put(index, ' ', RESET);
        }
    }
    index = 0;
    color = RESET;
}

void vga_put(int index, char c, unsigned char attr) {
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

/*
 * MANAGED ACCESS
 */

int vga_cursor_row() {
    return vga_row(index);
}

int vga_cursor_col() {
    return vga_col(index);
}

void vga_cursor(int row, int col) {
    if (row < 0 || col < 0 || row >= VGA_ROWS || col >= VGA_COLS)
        return;

    index = vga_index(row, col);
    update_cursor();
}

void vga_cursor_hide() {
    port_byte_out(REG_SCREEN_CTRL, 0x0a);
    port_byte_out(REG_SCREEN_DATA, 0x3f);
}

void vga_cursor_show() {
    port_byte_out(REG_SCREEN_CTRL, 0x0a);
    port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xc0) | 0xd);

    port_byte_out(REG_SCREEN_CTRL, 0x0b);
    port_byte_out(REG_SCREEN_DATA, (port_byte_in(REG_SCREEN_DATA) & 0xe0) | 0xe);
}

/*
 * HIGH LEVEL
 */

void vga_color(unsigned char attr) {
    color = attr;
}

size_t vga_putc(char c) {
    int ret = 0;
    if (c == '\n') {
        int row = vga_row(index);
        index = vga_index(row + 1, 0);
        ret = 0;
    }
    else if (c == '\b') {
        if (index > 0)
            index--;
        vga_put(index, ' ', RESET);
    }
    else {
        vga_put(index++, c, color);
        ret = 1;
    }

    if (index >= MAX_INDEX) {
        shift_lines();
        index = vga_index(VGA_ROWS - 1, 0);
    }

    update_cursor();
    return ret;
}

size_t vga_print(const char * str) {
    if (!str)
        return 0;

    int len = 0;
    while (*str != 0) {
        vga_putc(*str++);
        len++;
    }
    return len;
}

/*
 * HELPER FUNCTIONS
 */

static void update_cursor() {
    if (index < 0 || index >= MAX_INDEX)
        return;

    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index & 0xff));
}

static void shift_lines() {
    char * screen = (char *)VGA_ADDRESS;
    memmove(screen, screen + (VGA_COLS * 2), ((VGA_ROWS - 1) * VGA_COLS * 2));
    for (int col = 0; col < VGA_COLS; col++) {
        int index = vga_index(VGA_ROWS - 1, col);
        vga_put(index, ' ', RESET);
    }
}
