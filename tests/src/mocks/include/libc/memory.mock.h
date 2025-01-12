#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/memory.h"

DECLARE_FAKE_VALUE_FUNC(void *, kmalloc, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, krealloc, void *, size_t);
DECLARE_FAKE_VOID_FUNC(kfree, void *);

void reset_libc_memory_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
