#include "../drivers/vga.h"
#include "../drivers/ports.h"

// TERM TO PRINT TEXT AND COLOR

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

static void set_cursor_index(int index) {
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index & 0xff));
}

typedef struct {
    int row, col;
    int w, h;
    unsigned char color;
    unsigned char * screen;
} tty_t;

tty_t term;

void init_term() {
    term.row = 0;
    term.col = 0;
    term.color = VGA_WHITE_ON_BLACK;
}

void test1() {
    int i = VGA_COLS * 2;
    for (char l = 'A'; l <= 'Z'; l++) {
        vga_put(i, l, VGA_WHITE_ON_BLACK);
        i++;
    }
}

void test2() {
    set_cursor_index(0);
}

void main() {
    vga_clear();
    //init_term();
    //clear_term();
    test1();
    test2();
    // clear_screen();
    // kput_at('X', 0, 0);
    // kput_at('X', 0, 1);
    // kprint_at("YY\nZZ", 1, 0);
    // kprint("Hello\nWorld");
    // kprint_at("Hi", 3, 3);
    // kput_at('H', 5, 5);
    // kput_at('e', 5, 6);
    // kprint_at("Fail\nBad", MAX_ROWS, MAX_COLS);
}
