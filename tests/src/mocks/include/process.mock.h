#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "process.h"

DECLARE_FAKE_VALUE_FUNC(int, process_create, process_t *);
DECLARE_FAKE_VALUE_FUNC(int, process_free, process_t *);
DECLARE_FAKE_VALUE_FUNC(void *, process_add_pages, process_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(int, process_grow_stack, process_t *);
DECLARE_FAKE_VALUE_FUNC(int, process_load_heap, process_t *, const char *, size_t);
DECLARE_FAKE_VOID_FUNC(set_next_pid, uint32_t);

void reset_process_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
