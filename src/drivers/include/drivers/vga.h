#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

#define VGA_ROWS           25
#define VGA_COLS           80
#define RESET              (VGA_FG_LIGHT_GRAY | VGA_BG_BLACK)
#define VGA_WHITE_ON_BLACK (VGA_FG_WHITE | VGA_BG_BLACK)
#define VGA_RED_ON_WHITE   (VGA_FG_RED | VGA_BG_WHITE)

enum VGA_FG {
    VGA_FG_BLACK         = 0x0,
    VGA_FG_BLUE          = 0x1,
    VGA_FG_GREEN         = 0x2,
    VGA_FG_CYAN          = 0x3,
    VGA_FG_RED           = 0x4,
    VGA_FG_MAGENTA       = 0x5,
    VGA_FG_BROWN         = 0x6,
    VGA_FG_LIGHT_GRAY    = 0x7,
    VGA_FG_DARK_GRAY     = 0x8,
    VGA_FG_LIGHT_BLUE    = 0x9,
    VGA_FG_LIGHT_GREEN   = 0xA,
    VGA_FG_LIGHT_CYAN    = 0xB,
    VGA_FG_LIGHT_RED     = 0xC,
    VGA_FG_LIGHT_MAGENTA = 0xD,
    VGA_FG_LIGHT_BROWN   = 0xE,
    VGA_FG_WHITE         = 0xF,
};

enum VGA_BG {
    VGA_BG_BLACK         = 0x00,
    VGA_BG_BLUE          = 0x10,
    VGA_BG_GREEN         = 0x20,
    VGA_BG_CYAN          = 0x30,
    VGA_BG_RED           = 0x40,
    VGA_BG_MAGENTA       = 0x50,
    VGA_BG_BROWN         = 0x60,
    VGA_BG_LIGHT_GRAY    = 0x70,
    VGA_BG_DARK_GRAY     = 0x80,
    VGA_BG_LIGHT_BLUE    = 0x90,
    VGA_BG_LIGHT_GREEN   = 0xA0,
    VGA_BG_LIGHT_CYAN    = 0xB0,
    VGA_BG_LIGHT_RED     = 0xC0,
    VGA_BG_LIGHT_MAGENTA = 0xD0,
    VGA_BG_LIGHT_BROWN   = 0xE0,
    VGA_BG_WHITE         = 0xF0,
};

void vga_clear();
void vga_put(int index, char c, unsigned char attr);
int  vga_row(int index);
int  vga_col(int index);
int  vga_index(int row, int col);

int  vga_cursor_row();
int  vga_cursor_col();
void vga_cursor(int row, int col);
void vga_cursor_hide();
void vga_cursor_show();

void   vga_color(unsigned char color);
size_t vga_putc(char c);
size_t vga_print(const char * str);

size_t vga_puti(int num);
size_t vga_putu(unsigned int num);
size_t vga_putx(unsigned int num);

#endif // VGA_H
