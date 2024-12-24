#ifndef LIBC_SIGNAL_H
#define LIBC_SIGNAL_H

#include <stddef.h>

enum PROC_SIGNALS {
    PROC_SIGNALS_FOO = 1,
};

typedef void (*signal_handler)(void);

int register_signal(int sig_no, signal_handler callback);

#endif // LIBC_SIGNAL_H
