#ifndef LIBC_PROC_H
#define LIBC_PROC_H

#include <stdint.h>

#include "debug.h"
#include "ebus.h"

#ifdef TESTING
#define NO_RETURN
#else
#define NO_RETURN _Noreturn
#endif

#define PANIC(MSG) proc_panic((MSG), __FILE__, __LINE__)

#define kassert(CHECK)                     \
    if (!(CHECK)) {                        \
        PANIC("assertion failed " #CHECK); \
    }
#define kassert_msg(CHECK, MSG)                      \
    if (!(CHECK)) {                                  \
        PANIC("assertion failed " #CHECK " : " MSG); \
    }

void           proc_exit(uint8_t code);
void           proc_abort(uint8_t code, const char * msg);
NO_RETURN void proc_panic(const char * msg, const char * file, unsigned int line);

void queue_event(ebus_event_t * event);
int  yield(int filter);

int getpid(void);

#endif // LIBC_PROC_H
