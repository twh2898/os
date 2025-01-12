#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "libk/sys_call.mock.h"

// libk/sys_call.h

DEFINE_FAKE_VALUE_FUNC(void *, _page_alloc, size_t);
DEFINE_FAKE_VOID_FUNC(_proc_exit, uint8_t);
DEFINE_FAKE_VOID_FUNC(_proc_abort, uint8_t, const char *);
DEFINE_FAKE_VOID_FUNC(_proc_panic, const char *, const char *, unsigned int);
DEFINE_FAKE_VOID_FUNC(_register_signals, void *);
DEFINE_FAKE_VALUE_FUNC(size_t, _putc, char);
DEFINE_FAKE_VALUE_FUNC(size_t, _puts, const char *);

void reset_libk_sys_call_mock(void) {
    RESET_FAKE(_page_alloc);
    RESET_FAKE(_proc_exit);
    RESET_FAKE(_proc_abort);
    RESET_FAKE(_proc_panic);
    RESET_FAKE(_register_signals);
    RESET_FAKE(_putc);
    RESET_FAKE(_puts);
}
