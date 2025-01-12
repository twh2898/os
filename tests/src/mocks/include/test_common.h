#pragma once

#ifdef __cplusplus
#include <gtest/gtest.h>

extern "C" {
#endif

#include <string.h>

#include "fff.h"
// no sort

// Copy definition from string.h to make vscode happy with fff
extern void * memcpy(void * __restrict __dest, const void * __restrict __src, size_t __n) __THROW __nonnull((1, 2));
extern void * memset(void * __s, int __c, size_t __n) __THROW __nonnull((1));

#include "cpu/mmu.mock.h"
#include "cpu/ports.mock.h"
#include "libc/memory.mock.h"
#include "libc/string.mock.h"
#include "memory_alloc.mock.h"
#include "paging.mock.h"
#include "ram.mock.h"

void init_mocks(void);

#ifdef __cplusplus
} // extern "C"
#endif
