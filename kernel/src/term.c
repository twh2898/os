#include "term.h"

#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "libc/stdio.h"
#include "libc/string.h"

char keybuff[4096] = {0};
size_t keybuff_i = 0;

static void exec_buff();

static void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        if (code == KEY_ENTER) {
            vga_putc(c);
            exec_buff();
            return;
        }

        if (code == KEY_BACKSPACE) {
            if (keybuff_i > 0) {
                keybuff_i--;
                vga_putc(c);
            }
            return;
        }

        if (keybuff_i >= sizeof(keybuff) - 1) {
            vga_color(VGA_FG_RED | VGA_BG_WHITE);
            vga_print("TERMINAL KEY BUFFER OVERFLOW!");
            return;
        }

        keybuff[keybuff_i++] = c;
        vga_putc(c);
    }
}

void term_init() {
    keyboard_set_cb(&key_cb);
    vga_print("> ");
}

static void exec_buff() {
    if (keybuff_i >= sizeof(keybuff) - 1) {
        keybuff_i = sizeof(keybuff) - 1;
    }
    keybuff[keybuff_i] = 0;
    printf("Got command %s\n", keybuff);

    keybuff_i = 0;
}
