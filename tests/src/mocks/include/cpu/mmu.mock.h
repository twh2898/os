#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu/mmu.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(mmu_dir_clear, mmu_dir_t *);
DECLARE_FAKE_VOID_FUNC(mmu_table_clear, mmu_table_t *);
DECLARE_FAKE_VALUE_FUNC(int, mmu_dir_set_addr, mmu_dir_t *, size_t, uint32_t);
DECLARE_FAKE_VALUE_FUNC(int, mmu_dir_set_flags, mmu_dir_t *, size_t, enum MMU_DIR_FLAG);
DECLARE_FAKE_VALUE_FUNC(int, mmu_dir_set, mmu_dir_t *, size_t, uint32_t, enum MMU_DIR_FLAG);
DECLARE_FAKE_VALUE_FUNC(uint32_t, mmu_dir_get_addr, mmu_dir_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(enum MMU_DIR_FLAG, mmu_dir_get_flags, mmu_dir_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(int, mmu_table_set_addr, mmu_table_t *, size_t, uint32_t);
DECLARE_FAKE_VALUE_FUNC(int, mmu_table_set_flags, mmu_table_t *, size_t, enum MMU_TABLE_FLAG);
DECLARE_FAKE_VALUE_FUNC(int, mmu_table_set, mmu_table_t *, size_t, uint32_t, enum MMU_TABLE_FLAG);
DECLARE_FAKE_VALUE_FUNC(uint32_t, mmu_table_get_addr, mmu_table_t *, size_t);
DECLARE_FAKE_VALUE_FUNC(enum MMU_TABLE_FLAG, mmu_table_get_flags, mmu_table_t *, size_t);
DECLARE_FAKE_VOID_FUNC(mmu_enable_paging, uint32_t);
DECLARE_FAKE_VOID_FUNC(mmu_disable_paging);
DECLARE_FAKE_VALUE_FUNC(bool, mmu_paging_enabled);
DECLARE_FAKE_VOID_FUNC(mmu_change_dir, uint32_t);
DECLARE_FAKE_VALUE_FUNC(uint32_t, mmu_get_curr_dir);

void reset_cpu_mmu_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
