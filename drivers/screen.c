#include "screen.h"
#include "ports.h"

static unsigned char *screen = (unsigned char *)VIDEO_ADDRESS;
static unsigned char color = WHITE_ON_BLACK;

static int get_cursor_offset();
static void set_cursor_offset(int offset);
static int get_offset(int row, int col);
static int get_offset_row(int offset);
static int get_offset_col(int offset);

// PUBLIC FUNCTIONS

void clear_screen() {
    int screen_size = MAX_ROWS * MAX_COLS;

    for (int i = 0; i < screen_size; i++) {
        screen[i*2] = ' ';
        screen[i*2 + 1] = WHITE_ON_BLACK;
    }
}

void set_screen_color(enum VGA_FG fg, enum VGA_BG bg) {
    color = fg | bg;
}

void kprint_at(char * message, int col, int row) {
    int offset;
    if (col >= 0 && row >= 0) {
        offset = get_offset(row, col);
    }
    else {
        offset = get_cursor_offset();
    }

    while (message != 0) {
        screen[offset*2] = *message;
        screen[offset*2 + 1] = color;
        offset++;
    }
    set_cursor_offset(offset);
}

void kprint(char * message) {
    kprint_at(message, -1, -1);
}

// LOCAL FUNCTIONS

static int get_cursor_offset() {
    port_byte_out(REG_SCREEN_CTRL, 14);

    int offset = port_byte_in(REG_SCREEN_DATA) << 8;

    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);

    return offset * 2;
}

static void set_cursor_offset(int offset) {
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

static int get_offset(int row, int col) {
    return (row * MAX_COLS) + col;
}

static int get_offset_row(int offset) {
    return offset / MAX_COLS;
}

static int get_offset_col(int offset) {
    return offset % MAX_COLS;
}
