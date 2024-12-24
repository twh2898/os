#ifndef LIBC_SIGNAL_H
#define LIBC_SIGNAL_H

#include <stddef.h>

typedef void (*signal_handler)(void);

int register_signal(int sig_no, signal_handler callback);

#endif // LIBC_SIGNAL_H
