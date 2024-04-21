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

static int echo_cmd(size_t argc, char ** argv) {
    bool next_line = true;
    if (argc > 1 && memcmp(argv[1], "-n", 2) == 0)
        next_line = false;

    size_t i = 1;
    if (!next_line)
        i++;
    for (; i < argc; i++) {
        puts(argv[i]);
        if (i < argc)
            putc(' ');
    }

    if (next_line)
        putc('\n');

    return 0;
}

static int debug_cmd(size_t argc, char ** argv) {
    debug = !debug;
    if (debug)
        puts("Enable debug\n");
    else
        puts("Disabling debug\n");
    return 0;
}

static int atoi_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        printf("%s requires one argument", argv[0]);
        return 1;
    }

    int i = atoi(argv[1]);
    printf("%d\n", i);
    return 0;
}

void commands_init() {
    term_command_add("clear", clear_cmd);
    term_command_add("echo", echo_cmd);
    term_command_add("debug", debug_cmd);
    term_command_add("atoi", atoi_cmd);
}
