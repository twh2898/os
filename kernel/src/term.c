#include "term.h"

#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define MAX_CHARS 4095
char keybuff[MAX_CHARS + 1] = {0};
size_t keybuff_i = 0;

#define MAX_COMMANDS 4096
command_t commands[MAX_COMMANDS] = {0};
size_t n_commands = 0;

int term_last_ret = 0;

static void exec_buff();
static char ** parse_args(char * line, size_t * out_len);

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

        if (keybuff_i > MAX_CHARS) {
            vga_color(VGA_RED_ON_WHITE);
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

void term_command_add(command_t command) {
    if (n_commands > MAX_COMMANDS) {
        vga_color(VGA_RED_ON_WHITE);
        vga_print("TERMINAL KEY BUFFER OVERFLOW!");
        return;
    }

    commands[n_commands++] = command;
}

static void exec_buff() {
    if (keybuff_i > MAX_CHARS) {
        keybuff_i = MAX_CHARS + 1;
    }
    keybuff[keybuff_i] = 0;
    printf("Got command line %s\n", keybuff);

    size_t space_i = strfind(keybuff, 0, ' ');
    for (size_t i = 0; i < n_commands; i++) {
        if (memcmp(keybuff, commands[i].command, space_i + 1) == 0) {
            // parse argv and argc
            size_t argc;
            char ** argv = parse_args(keybuff, &argc);
            if (!argv) {
                vga_color(VGA_RED_ON_WHITE);
                vga_print("Syntax Error");
                term_last_ret = 1;
            }
            term_last_ret = commands[i].cb(argc, argv);
        }
    }

    keybuff_i = 0;
}

static int count_args(char * line) {
    while (*line == ' ') line++;

    int n = 0;
    while (*line) {
        if (*line == '"') {
            int next = strfind(line, 0, '"');
            if (next == -1) {
                return -1;
            }
        }
    }
    return n;
}

static char ** parse_args(char * line, size_t * out_len) {
    int len = count_args(line);
    if (len < 0) {
        return 0;
    }
    *out_len = (size_t)len;
    while (*line == ' ') {
        line++;
    }
}
