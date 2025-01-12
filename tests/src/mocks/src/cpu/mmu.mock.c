#include "cpu/mmu.mock.h"

DEFINE_FAKE_VOID_FUNC(mmu_dir_clear, mmu_dir_t *);
DEFINE_FAKE_VOID_FUNC(mmu_table_clear, mmu_table_t *);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set_addr, mmu_dir_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set_flags, mmu_dir_t *, size_t, enum MMU_DIR_FLAG);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set, mmu_dir_t *, size_t, uint32_t, enum MMU_DIR_FLAG);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_dir_get_addr, mmu_dir_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(enum MMU_DIR_FLAG, mmu_dir_get_flags, mmu_dir_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set_addr, mmu_table_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set_flags, mmu_table_t *, size_t, enum MMU_TABLE_FLAG);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set, mmu_table_t *, size_t, uint32_t, enum MMU_TABLE_FLAG);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_table_get_addr, mmu_table_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(enum MMU_TABLE_FLAG, mmu_table_get_flags, mmu_table_t *, size_t);
DEFINE_FAKE_VOID_FUNC(mmu_enable_paging, mmu_dir_t *);
DEFINE_FAKE_VOID_FUNC(mmu_disable_paging);
DEFINE_FAKE_VALUE_FUNC(bool, mmu_paging_enabled);
DEFINE_FAKE_VOID_FUNC(mmu_change_dir, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_get_curr_dir);

void reset_mmu_mock() {
    RESET_FAKE(mmu_dir_clear);
    RESET_FAKE(mmu_table_clear);
    RESET_FAKE(mmu_dir_set_addr);
    RESET_FAKE(mmu_dir_set_flags);
    RESET_FAKE(mmu_dir_set);
    RESET_FAKE(mmu_dir_get_addr);
    RESET_FAKE(mmu_dir_get_flags);
    RESET_FAKE(mmu_table_set_addr);
    RESET_FAKE(mmu_table_set_flags);
    RESET_FAKE(mmu_table_set);
    RESET_FAKE(mmu_table_get_addr);
    RESET_FAKE(mmu_table_get_flags);
    RESET_FAKE(mmu_enable_paging);
    RESET_FAKE(mmu_disable_paging);
    RESET_FAKE(mmu_paging_enabled);
    RESET_FAKE(mmu_change_dir);
    RESET_FAKE(mmu_get_curr_dir);
}
