#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/memory.h"

DECLARE_FAKE_VALUE_FUNC(void *, malloc, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, realloc, void *, size_t);
DECLARE_FAKE_VOID_FUNC(free, void *);

void reset_libc_memory_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
