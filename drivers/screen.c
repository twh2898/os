#include "screen.h"

#include "ports.h"

static int put_char(char c, int row, int col, char color);
static int get_cursor_offset();
static void set_cursor_offset(int offset);
static int get_offset(int row, int col);
static int get_offset_row(int offset);
static int get_offset_col(int offset);

// PUBLIC FUNCTIONS

void clear_screen() {
    unsigned char * screen = (unsigned char *)VIDEO_ADDRESS;
    int screen_size = MAX_ROWS * MAX_COLS;

    for (int i = 0; i < screen_size; i++) {
        screen[i * 2] = ' ';
        screen[i * 2 + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(0);
}

void kput_at(char c, int row, int col) {
    put_char(c, row, col, WHITE_ON_BLACK);
}

void kprint_at(char * message, int col, int row) {
    unsigned char * screen = (unsigned char *)VIDEO_ADDRESS;

    int offset;
    if (col >= 0 && row >= 0) {
        offset = get_offset(row, col);
    }
    else {
        offset = get_cursor_offset();
    }

    int i;
    while (message[i] != 0) {
        offset = put_char(message[i++], col, row, WHITE_ON_BLACK);
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
    set_cursor_offset(offset);
}

void kprint(char * message) {
    kprint_at(message, -1, -1);
}

// LOCAL FUNCTIONS

/**
 * Put a single character at row, col or cursor position and return the new
 * offset. If row or col are -1 the cursor position will be used. If row or
 * col are greater than the max number of rows or columns, a red E will appear
 * in the lower right corner of the screen.
 */
static int put_char(char c, int row, int col, char attr) {
    unsigned char * screen = (unsigned char *)VIDEO_ADDRESS;

    if (!attr)
        attr = WHITE_ON_BLACK;

    if (row >= MAX_ROWS || col >= MAX_COLS) {
        screen[2 * (MAX_COLS) * (MAX_ROWS)-2] = 'E';
        screen[2 * (MAX_COLS) * (MAX_ROWS)] = RED_ON_WHITE;
        return get_offset(row, col);
    }

    int offset;
    if (col >= 0 && row >= 0)
        offset = get_offset(row, col);
    else
        offset = get_cursor_offset();

    if (c == '\n') {
        row = get_offset_row(offset);
        offset = get_offset(row + 1, 0);
    }
    else {
        screen[offset * 2] = c;
        screen[offset * 2 + 1] = attr;
        offset += 2;
    }

    set_cursor_offset(offset);
    return offset;
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
    return 2 * (row * MAX_COLS + col);
}

static int get_offset_row(int offset) {
    return offset / (2 * MAX_COLS);
}

static int get_offset_col(int offset) {
    int row = get_offset_row(offset);
    return (offset - (row * 2 * MAX_COLS)) / 2;
}
