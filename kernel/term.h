#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../drivers/vga.h"

void term_init();

int term_cursor_row();
int term_cursor_col();
void term_cursor(int row, int col);
void term_cursor_hide();
void term_cursor_show();

void term_color(unsigned char color);

size_t term_putc(char c);

size_t term_print(const char * str);

#endif // TERM_H
