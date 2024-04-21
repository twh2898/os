#include "commands.h"

#include "drivers/vga.h"
#include "term.h"

static int clear_cmd(const char * line) {
    vga_clear();
    return 0;
}

static command_t command_clear = {
    .command = "clear",
    .cb = clear_cmd,
};

static int echo_cmd(const char * line) {
    vga_clear();
    return 0;
}

static command_t echo_clear = {
    .command = "echo",
    .cb = clear_cmd,
};

void commands_init() {
    term_command_add(command_clear);
    term_command_add(echo_clear);
}
