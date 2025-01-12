#pragma once

#ifdef __cplusplus
#include <gtest/gtest.h>

extern "C" {
#endif

#include <string.h>

#include "fff.h"
// no sort

// Copy definition from string.h to make vscode happy with fff
extern void * memcpy(void *, const void *, size_t);
extern void * memset(void *, int, size_t);

#include "cpu/mmu.mock.h"
#include "cpu/ports.mock.h"
#include "libc/memory.mock.h"
#include "libc/string.mock.h"
#include "libk/sys_call.mock.h"
#include "memory_alloc.mock.h"
#include "paging.mock.h"
#include "ram.mock.h"

void init_mocks(void);

#ifdef __cplusplus
} // extern "C"
#endif
