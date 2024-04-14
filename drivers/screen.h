#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define MAX_OFFSET (MAX_ROWS * MAX_COLS * 2)
#define WHITE_ON_BLACK (VGA_FG_WHITE | VGA_BG_BLACK)
#define GRAY_ON_BLACK (VGA_FG_LIGHT_GRAY | VGA_BG_BLACK)
#define BLACK_ON_WHITE (VGA_FG_BLACK | VGA_BG_WHITE)
#define RED_ON_WHITE (VGA_FG_RED | VGA_BG_WHITE)

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void clear_screen();
void kput_at(char c, int row, int col);
void kprint_at(char * message, int row, int col);
void kprint(char * message);

#endif // SCREEN_H
