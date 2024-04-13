#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK (VGA_FG_WHITE | VGA_BG_BLACK)
#define BLACK_ON_WHITE (VGA_FG_BLACK | VGA_BG_WHITE)
#define RED_ON_WHITE (VGA_FG_RED | VGA_BG_WHITE)

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

enum VGA_FG {
    VGA_FG_BLACK = 0x0,
    VGA_FG_BLUE = 0x1,
    VGA_FG_GREEN = 0x2,
    VGA_FG_CYAN = 0x3,
    VGA_FG_RED = 0x4,
    VGA_FG_MAGENTA = 0x5,
    VGA_FG_BROWN = 0x6,
    VGA_FG_LIGHT_GREY = 0x7,
    VGA_FG_DARK_GREY = 0x8,
    VGA_FG_LIGHT_BLUE = 0x9,
    VGA_FG_LIGHT_GREEN = 0xA,
    VGA_FG_LIGHT_CYAN = 0xB,
    VGA_FG_LIGHT_RED = 0xC,
    VGA_FG_LIGHT_MAGENTA = 0xD,
    VGA_FG_LIGHT_BROWN = 0xE,
    VGA_FG_WHITE = 0xF,
};

enum VGA_BG {
    VGA_BG_BLACK = 0x00,
    VGA_BG_BLUE = 0x10,
    VGA_BG_GREEN = 0x20,
    VGA_BG_CYAN = 0x30,
    VGA_BG_RED = 0x40,
    VGA_BG_MAGENTA = 0x50,
    VGA_BG_BROWN = 0x60,
    VGA_BG_LIGHT_GREY = 0x70,
    VGA_BG_DARK_GREY = 0x80,
    VGA_BG_LIGHT_BLUE = 0x90,
    VGA_BG_LIGHT_GREEN = 0xA0,
    VGA_BG_LIGHT_CYAN = 0xB0,
    VGA_BG_LIGHT_RED = 0xC0,
    VGA_BG_LIGHT_MAGENTA = 0xD0,
    VGA_BG_LIGHT_BROWN = 0xE0,
    VGA_BG_WHITE = 0xF0,
};

void clear_screen();
void set_screen_color(enum VGA_FG fg, enum VGA_BG bg);
void kput_at(char c, int col, int row);
void kprint_at(char * message, int col, int row);
void kprint(char * message);

#endif // SCREEN_H
