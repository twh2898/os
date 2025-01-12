#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "memory_alloc.h"

DECLARE_FAKE_VALUE_FUNC(int, memory_init, memory_t *, memory_alloc_pages_t);
DECLARE_FAKE_VALUE_FUNC(void *, memory_alloc, memory_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, memory_realloc, memory_t *, void *, size_t);
DECLARE_FAKE_VALUE_FUNC(int, memory_free, memory_t *, void *);
DECLARE_FAKE_VALUE_FUNC(int, memory_split_entry, memory_t *, memory_entry_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(int, memory_merge_with_next, memory_t *, memory_entry_t *);
DECLARE_FAKE_VALUE_FUNC(memory_entry_t *, memory_find_entry_size, memory_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(memory_entry_t *, memory_find_entry_ptr, memory_t *, void *);
DECLARE_FAKE_VALUE_FUNC(memory_entry_t *, memory_add_entry, memory_t *, size_t);

void reset_memory_alloc_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
