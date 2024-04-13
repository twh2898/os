#include "../drivers/screen.h"

void main() {
    clear_screen();
    kput_at('X', 0, 0);
    kput_at('X', 0, 1);
    kprint_at("YY\nZZ", 0, 2);
    //kprint("Hello\nWorld");
    //kprint_at("Hi", 3, 3);
    //kput_at('H', 5, 5);
    //kput_at('e', 5, 6);
    //kprint_at("Fail\nBad", MAX_ROWS, MAX_COLS);
}
