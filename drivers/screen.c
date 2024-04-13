#include "screen.h"
#include "ports.h"

static int put_char(char c, int row, int col);
static int put_char(char c, int row, int col, char color);
static int get_cursor_offset();
static void set_cursor_offset(int offset);
static int get_offset(int row, int col);
static int get_offset_row(int offset);
static int get_offset_col(int offset);

// PUBLIC FUNCTIONS

void clear_screen() {
    char * screen = (char *)VIDEO_ADDRESS;
    int screen_size = MAX_ROWS * MAX_COLS;

    for (int i = 0; i < screen_size; i++) {
        screen[i*2] = ' ';
        screen[i*2 + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(0);
}

void set_screen_color(enum VGA_FG fg, enum VGA_BG bg) {
    color = fg | bg;
}

void kput_at(char c, int row, int col) {
    int offset = get_offset(row, col);
    char * screen = (char *) VIDEO_ADDRESS;
    screen[offset * 2] = c;
    screen[offset * 2 + 1] = WHITE_ON_BLACK;
}

void kprint_at(char * message, int col, int row) {
    char * screen = (char *)VIDEO_ADDRESS;

    if (col >= 0 && row >= 0) {
        offset = get_offset(row, col);
    }
    else {
        offset = get_cursor_offset();
    }

    while (*message != 0) {
        screen[offset*2] = *message;
        screen[offset*2 + 1] = color;
        offset++;
        message++;
    }
    set_cursor_offset(offset);
}

void kprint(char * message) {
    kprint_at(message, -1, -1);
}

// LOCAL FUNCTIONS

static int put_char(char c, int row, int col) {
    return put_char(c, row, col, WHITE_ON_BLACK);
}

static int put_char(char c, int row, int col, char attr) {
    char * screen = (char *) VIDEO_ADDRESS;

    if (!attr)
        attr = WHITE_ON_BLACK;
    
    if (row >= MAX_ROWS || col >= MAX_COLS) {
        screen[2*(MAX_COLS)*(MAX_ROWS)-2] = 'E';
        screen[2*(MAX_COLS)*(MAX_ROWS)] = RED_ON_WHITE;
        return get_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0)
        offset = get_offset(col, row);
    else
        offset = get_cursor_offset();

    screen[offset * 2] = c;
    screen[offset * 2 + 1] = attr;
}

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
