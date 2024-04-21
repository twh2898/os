#include "commands.h"

#include <stdbool.h>
#include <stddef.h>

#include "debug.h"
#include "drivers/vga.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"

bool debug = false;

static int clear_cmd(size_t argc, char ** argv) {
    vga_clear();
    return 0;
}

static command_t command_clear = {
    .command = "clear",
    .cb = clear_cmd,
};

static int echo_cmd(size_t argc, char ** argv) {
    bool newline = true;
    if (argc > 1 && memcmp(argv[1], "-n", 2) == 0)
        newline = false;

    size_t i = 1;
    if (!newline)
        i++;
    for (; i < argc; i++) {
        puts(argv[i]);
        if (i < argc)
            putc(' ');
    }

    if (newline)
        putc('\n');

    return 0;
}

static command_t command_echo = {
    .command = "echo",
    .cb = echo_cmd,
};

static int debug_cmd(size_t argc, char ** argv) {
    debug = true;
    puts("Enable debug\n");
    return 0;
}

static command_t command_debug = {
    .command = "debug",
    .cb = debug_cmd,
};

void commands_init() {
    term_command_add(command_clear);
    term_command_add(command_echo);
    term_command_add(command_debug);
}
