#include "cpu/mmu.mock.h"
#include "cpu/ports.mock.h"
#include "cpu/tss.mock.h"

// cpu/mmu.h

DEFINE_FAKE_VOID_FUNC(mmu_dir_clear, mmu_dir_t *);
DEFINE_FAKE_VOID_FUNC(mmu_table_clear, mmu_table_t *);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set_addr, mmu_dir_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set_flags, mmu_dir_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_dir_set, mmu_dir_t *, size_t, uint32_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_dir_get_addr, mmu_dir_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_dir_get_flags, mmu_dir_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set_addr, mmu_table_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set_flags, mmu_table_t *, size_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(int, mmu_table_set, mmu_table_t *, size_t, uint32_t, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_table_get_addr, mmu_table_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_table_get_flags, mmu_table_t *, size_t);
DEFINE_FAKE_VOID_FUNC(mmu_flush_tlb, uint32_t);
DEFINE_FAKE_VOID_FUNC(mmu_enable_paging, uint32_t);
DEFINE_FAKE_VOID_FUNC(mmu_disable_paging);
DEFINE_FAKE_VALUE_FUNC(bool, mmu_paging_enabled);
DEFINE_FAKE_VOID_FUNC(mmu_change_dir, uint32_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, mmu_get_curr_dir);
DEFINE_FAKE_VOID_FUNC(mmu_reload_dir);

void reset_cpu_mmu_mock() {
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
    RESET_FAKE(mmu_flush_tlb);
    RESET_FAKE(mmu_enable_paging);
    RESET_FAKE(mmu_disable_paging);
    RESET_FAKE(mmu_paging_enabled);
    RESET_FAKE(mmu_change_dir);
    RESET_FAKE(mmu_get_curr_dir);
    RESET_FAKE(mmu_reload_dir);
}

// cpu/ports.h

DEFINE_FAKE_VALUE_FUNC(uint8_t, port_byte_in, uint16_t);
DEFINE_FAKE_VOID_FUNC(port_byte_out, uint16_t, uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t, port_word_in, uint16_t);
DEFINE_FAKE_VOID_FUNC(port_word_out, uint16_t, uint16_t);

void reset_cpu_ports_mock() {
    RESET_FAKE(port_byte_in);
    RESET_FAKE(port_byte_out);
    RESET_FAKE(port_word_in);
    RESET_FAKE(port_word_out);
}

// cpu/tss.h

DEFINE_FAKE_VOID_FUNC(init_tss);
DEFINE_FAKE_VALUE_FUNC(tss_entry_t *, tss_get_entry, size_t);
DEFINE_FAKE_VOID_FUNC(tss_set_esp0, uint32_t);

void reset_cpu_tss_mock() {
    RESET_FAKE(init_tss);
    RESET_FAKE(tss_get_entry);
    RESET_FAKE(tss_set_esp0);
}
