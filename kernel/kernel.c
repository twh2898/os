#include "../drivers/screen.h"

void main() {
    clear_screen();
    kprint("Hello World");
    kprint_at("Hi", 3, 3);
}
