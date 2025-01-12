
#include "ram.mock.h"

DEFINE_FAKE_VOID_FUNC(ram_init, void *, size_t *);
DEFINE_FAKE_VALUE_FUNC(uint16_t, ram_lower_size);
DEFINE_FAKE_VALUE_FUNC(uint16_t, ram_upper_count);
DEFINE_FAKE_VALUE_FUNC(uint64_t, ram_upper_start, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint64_t, ram_upper_end, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint64_t, ram_upper_size, uint16_t);
DEFINE_FAKE_VALUE_FUNC(bool, ram_upper_usable, uint16_t);
DEFINE_FAKE_VALUE_FUNC(enum RAM_TYPE, ram_upper_type, uint16_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_bitmask_paddr, size_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_bitmask_vaddr, size_t);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_page_alloc);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_page_palloc);
DEFINE_FAKE_VOID_FUNC(ram_page_free, uint32_t);

void reset_ram_mock() {
    RESET_FAKE(ram_init);
    RESET_FAKE(ram_lower_size);
    RESET_FAKE(ram_upper_count);
    RESET_FAKE(ram_upper_start);
    RESET_FAKE(ram_upper_end);
    RESET_FAKE(ram_upper_size);
    RESET_FAKE(ram_upper_usable);
    RESET_FAKE(ram_upper_type);
    RESET_FAKE(ram_bitmask_paddr);
    RESET_FAKE(ram_bitmask_vaddr);
    RESET_FAKE(ram_page_alloc);
    RESET_FAKE(ram_page_palloc);
    RESET_FAKE(ram_page_free);
}
