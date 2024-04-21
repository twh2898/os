#include "commands.h"

#include "drivers/vga.h"
#include "libc/stdio.h"
#include "term.h"

static int clear_cmd(size_t argc, char ** argv) {
    vga_clear();
    return 0;
}

static command_t command_clear = {
    .command = "clear",
    .cb = clear_cmd,
};

static int echo_cmd(size_t argc, char ** argv) {
    printf("Echo %u args\n", argc);
    for (size_t i = 0; i < argc; i++) {
        printf("Arg %u is %s\n", i, argv[i]);
    }
    return 0;
}

static command_t echo_clear = {
    .command = "echo",
    .cb = echo_cmd,
};

void commands_init() {
    term_command_add(command_clear);
    term_command_add(echo_clear);
}
