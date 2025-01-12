#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/string.h"

DECLARE_FAKE_VALUE_FUNC(int, kmemcmp, const void *, const void *, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, kmemcpy, void *, const void *, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, kmemmove, void *, const void *, size_t);
DECLARE_FAKE_VALUE_FUNC(void *, kmemset, void *, int, size_t);
DECLARE_FAKE_VALUE_FUNC(size_t, kstrlen, const char *);
DECLARE_FAKE_VALUE_FUNC(size_t, knstrlen, const char *, int);
DECLARE_FAKE_VALUE_FUNC(int, kstrcmp, const char *, const char *);
DECLARE_FAKE_VALUE_FUNC(char *, kstrfind, const char *, int);
DECLARE_FAKE_VALUE_FUNC(char *, kstrtok, char *, const char *);
DECLARE_FAKE_VALUE_FUNC(int, katoi, const char *);

void reset_libc_string_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
