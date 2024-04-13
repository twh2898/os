#include "../drivers/screen.h"

void main() {
    clear_screen();
    kprint("Hello World");
    set_screen_color(VGA_FG_WHITE, VGA_BG_BLACK);
    kprint_at("Hi", 3, 3);
    while (1);
}
