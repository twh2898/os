#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "paging.h"

DECLARE_FAKE_VOID_FUNC(paging_init);
DECLARE_FAKE_VALUE_FUNC(void *, paging_temp_map, uint32_t);
DECLARE_FAKE_VOID_FUNC(paging_temp_free, uint32_t);
DECLARE_FAKE_VALUE_FUNC(size_t, paging_temp_available);
DECLARE_FAKE_VALUE_FUNC(int, paging_id_map_range, size_t, size_t);
DECLARE_FAKE_VALUE_FUNC(int, paging_id_map_page, size_t);
DECLARE_FAKE_VALUE_FUNC(int, paging_add_pages, uint32_t, size_t, size_t);
DECLARE_FAKE_VALUE_FUNC(int, paging_remove_pages, uint32_t, size_t, size_t);
DECLARE_FAKE_VALUE_FUNC(int, paging_add_table, uint32_t, size_t);
DECLARE_FAKE_VALUE_FUNC(int, paging_remove_table, uint32_t, size_t);

void reset_paging_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
