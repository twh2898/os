#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libk/sys_call.h"

DECLARE_FAKE_VALUE_FUNC(void *, _page_alloc, size_t);
DECLARE_FAKE_VOID_FUNC(_proc_exit, uint8_t);
DECLARE_FAKE_VOID_FUNC(_proc_abort, uint8_t, const char *);
DECLARE_FAKE_VOID_FUNC(_proc_panic, const char *, const char *, unsigned int);
DECLARE_FAKE_VOID_FUNC(_register_signals, void *);
DECLARE_FAKE_VALUE_FUNC(size_t, _putc, char);
DECLARE_FAKE_VALUE_FUNC(size_t, _puts, const char *);

void reset_libk_sys_call_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
