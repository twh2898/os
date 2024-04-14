# ifndef TERM_H
#define TERM_H

#include "../drivers/vga.h"

void term_init();
void term_putc(char c);
void term_print(const char * str);
void term_cursor(int row, int col);
void term_color(unsigned char color);
void shift_lines();

#endif // TERM_H
