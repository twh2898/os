
#include "memory_alloc.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, memory_init, memory_t *, memory_alloc_pages_t);
DEFINE_FAKE_VALUE_FUNC(void *, memory_alloc, memory_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, memory_realloc, memory_t *, void *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, memory_free, memory_t *, void *);
DEFINE_FAKE_VALUE_FUNC(int, memory_split_entry, memory_t *, memory_entry_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, memory_merge_with_next, memory_t *, memory_entry_t *);
DEFINE_FAKE_VALUE_FUNC(memory_entry_t *, memory_find_entry_size, memory_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(memory_entry_t *, memory_find_entry_ptr, memory_t *, void *);
DEFINE_FAKE_VALUE_FUNC(memory_entry_t *, memory_add_entry, memory_t *, size_t);

void reset_memory_alloc_mock() {
    RESET_FAKE(memory_init);
    RESET_FAKE(memory_alloc);
    RESET_FAKE(memory_realloc);
    RESET_FAKE(memory_free);
    RESET_FAKE(memory_split_entry);
    RESET_FAKE(memory_merge_with_next);
    RESET_FAKE(memory_find_entry_size);
    RESET_FAKE(memory_find_entry_ptr);
    RESET_FAKE(memory_add_entry);
}
