#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "ram.h"

DECLARE_FAKE_VOID_FUNC(ram_init, void *, size_t *);
DECLARE_FAKE_VALUE_FUNC(uint16_t, ram_lower_size);
DECLARE_FAKE_VALUE_FUNC(uint16_t, ram_upper_count);
DECLARE_FAKE_VALUE_FUNC(uint64_t, ram_upper_start, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint64_t, ram_upper_end, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint64_t, ram_upper_size, uint16_t);
DECLARE_FAKE_VALUE_FUNC(bool, ram_upper_usable, uint16_t);
DECLARE_FAKE_VALUE_FUNC(enum RAM_TYPE, ram_upper_type, uint16_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, ram_bitmask_paddr, size_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, ram_bitmask_vaddr, size_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, ram_page_alloc);
DECLARE_FAKE_VALUE_FUNC(uint32_t, ram_page_palloc);
DECLARE_FAKE_VOID_FUNC(ram_page_free, uint32_t);

void reset_ram_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
