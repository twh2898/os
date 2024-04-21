#ifndef TERM_H
#define TERM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*command_cb_t)(const char * line);

typedef struct {
    const char * command;
    command_cb_t cb;
} command_t;

extern int term_last_ret;

void term_init();

void term_command_add(command_t command);

#endif // TERM_H
