#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/datastruct/array.h"

DECLARE_FAKE_VALUE_FUNC(int, arr_create, arr_t *, size_t, size_t);
DECLARE_FAKE_VOID_FUNC(arr_free, arr_t *);
DECLARE_FAKE_VALUE_FUNC(size_t, arr_size, const arr_t *);
DECLARE_FAKE_VALUE_FUNC(void *, arr_data, const arr_t *);
DECLARE_FAKE_VALUE_FUNC(void *, arr_at, const arr_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(int, arr_set, arr_t *, size_t, const void *);
DECLARE_FAKE_VALUE_FUNC(int, arr_get, const arr_t *, size_t, void *);
DECLARE_FAKE_VALUE_FUNC(int, arr_insert, arr_t *, size_t, const void *);
DECLARE_FAKE_VALUE_FUNC(int, arr_remove, arr_t *, size_t, void *);

void reset_libc_datastruct_array_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
