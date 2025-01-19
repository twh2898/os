
#include "process.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, process_create, process_t *);
DEFINE_FAKE_VALUE_FUNC(int, process_free, process_t *);
DEFINE_FAKE_VALUE_FUNC(void *, process_add_pages, process_t *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, process_grow_stack, process_t *);
DEFINE_FAKE_VOID_FUNC(set_next_pid, uint32_t);

void reset_process_mock() {
    RESET_FAKE(process_create);
    RESET_FAKE(process_free);
    RESET_FAKE(process_add_pages);
    RESET_FAKE(process_grow_stack);
    RESET_FAKE(set_next_pid);
}
