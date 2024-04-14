#include "term.h"

#include "../drivers/ports.h"
#include "../drivers/vga.h"
#include "std.h"

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

void term_cursor(int row, int col) {
    if (row < 0 || col < 0 || row >= VGA_ROWS || col >= VGA_COLS)
        return;

    index = vga_index(row, col);
    update_cursor();
}

void term_color(unsigned char attr) {
    color = attr;
}

void term_putc(char c) {
    if (c == '\n') {
        int row = vga_row(index);
        index = vga_index(row + 1, 0);
    }
    else {
        vga_put(index++, c, color);
    }

    if (index >= MAX_INDEX) {
        shift_lines();
        index = vga_index(VGA_ROWS - 1, 0);
    }

    update_cursor();
}

int term_print(const char * str) {
    int len = 0;
    while (*str != 0) {
        term_putc(*str++);
        len++;
    }
    return len;
}

static char digit(unsigned int num, int base, bool upper) {
    if (num < 10)
        return num + '0';
    else
        return (num - 10) + (upper ? 'A' : 'a');
}

int term_puti(int num, int base, bool upper) {
    if (num == 0) {
        term_putc('0');
        return 1;
    }

    bool is_neg = num < 0;

    if (num < 0) {
        term_putc('-');
        num = -num;
    }

    int len = 0;
    int rev = 0;
    while (num > 0) {
        rev = (rev * base) + (num % base);
        num /= base;
        len++;
    }

    for (int i = 0; i < len; i++) {
        term_putc(digit(rev % base, base, upper));
        rev /= base;
    }

    if (is_neg)
        len++;

    return len;
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
    char * screen = (char *)VGA_ADDRES;
    memmove(screen, screen + (VGA_COLS * 2), ((VGA_ROWS - 1) * VGA_COLS * 2));
    for (int col = 0; col < VGA_COLS; col++) {
        int index = vga_index(VGA_ROWS - 1, col);
        vga_put(index, ' ', VGA_WHITE_ON_BLACK);
    }
}
