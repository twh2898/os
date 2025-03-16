
#include "ram.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, ram_init, ram_table_t *, void *);
DEFINE_FAKE_VALUE_FUNC(int, ram_region_add_memory, uint64_t, uint64_t);
DEFINE_FAKE_VALUE_FUNC(size_t, ram_free_pages);
DEFINE_FAKE_VALUE_FUNC(size_t, ram_max_pages);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_page_alloc);
DEFINE_FAKE_VALUE_FUNC(uint32_t, ram_page_palloc);
DEFINE_FAKE_VALUE_FUNC(int, ram_page_free, uint32_t);

void reset_ram_mock() {
    RESET_FAKE(ram_init);
    RESET_FAKE(ram_region_add_memory);
    RESET_FAKE(ram_free_pages);
    RESET_FAKE(ram_max_pages);
    RESET_FAKE(ram_page_alloc);
    RESET_FAKE(ram_page_palloc);
    RESET_FAKE(ram_page_free);
}
