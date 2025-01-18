
#include "memory_alloc.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, memory_init, memory_t *, memory_alloc_pages_t);
DEFINE_FAKE_VALUE_FUNC(void *, memory_alloc, memory_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, memory_realloc, memory_t *, void *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, memory_free, memory_t *, void *);

void reset_memory_alloc_mock() {
    RESET_FAKE(memory_init);
    RESET_FAKE(memory_alloc);
    RESET_FAKE(memory_realloc);
    RESET_FAKE(memory_free);
}
