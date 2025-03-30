#include "commands.h"

#include "libc/proc.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "shell.h"

static int echo_cmd(size_t argc, char ** argv) {
    bool next_line = true;
    if (argc > 1 && kmemcmp(argv[1], "-n", 2) == 0) {
        next_line = false;
    }

    size_t i = 1;
    if (!next_line) {
        i++;
    }
    for (; i < argc; i++) {
        puts(argv[i]);
        if (i < argc) {
            putc(' ');
        }
    }

    if (next_line) {
        putc('\n');
    }

    return 0;
}

void init_commands() {
    term_command_add("echo", echo_cmd);
}
