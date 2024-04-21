#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*command_cb_t)(size_t argc, char ** argv);

extern int term_last_ret;

void term_init();

void term_command_add(const char * command, command_cb_t cb);

#endif // TERM_H
