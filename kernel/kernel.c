#include "../drivers/screen.h"

void main() {
    clear_screen();
    kprint("Hello World\r\n");
    kprint_at("Hi", 3, 3);
}
