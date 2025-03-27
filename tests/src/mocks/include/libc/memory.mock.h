#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/memory.h"

DECLARE_FAKE_VALUE_FUNC(void *, pmalloc, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, prealloc, void *, size_t);
DECLARE_FAKE_VOID_FUNC(pfree, void *);

void reset_libc_memory_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
