#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*command_cb_t)(size_t argc, char ** argv);

extern int term_last_ret;

void term_init();

void term_update();

void term_run();

bool term_command_add(const char * command, command_cb_t cb);

void set_command_lookup(command_cb_t command);

#endif // TERM_H
