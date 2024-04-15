#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stdint.h>

#include "../drivers/vga.h"

void term_init();
void term_cursor(int row, int col);
void term_cursor_hide();
void term_color(unsigned char color);
void term_putc(char c);
int term_puti(int num, int base, bool upper);
int term_putu(unsigned int num, unsigned int base, bool upper);
int term_print(const char * str);

#endif // TERM_H
