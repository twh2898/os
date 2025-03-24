#include "ebus.mock.h"

// ebus.h

DEFINE_FAKE_VALUE_FUNC(int, ebus_create, ebus_t *, size_t);
DEFINE_FAKE_VOID_FUNC(ebus_free, ebus_t *);
DEFINE_FAKE_VALUE_FUNC(int, ebus_queue_size, ebus_t *);
DEFINE_FAKE_VALUE_FUNC(int, ebus_register_handler, ebus_t *, ebus_handler_t *);
DEFINE_FAKE_VOID_FUNC(ebus_uregister_handler, ebus_t *, int);
DEFINE_FAKE_VALUE_FUNC(int, ebus_push, ebus_t *, ebus_event_t *);
DEFINE_FAKE_VALUE_FUNC(int, ebus_pop, ebus_t *, ebus_event_t *);
DEFINE_FAKE_VALUE_FUNC(int, ebus_cycle, ebus_t *);

void reset_ebus_mock() {
    RESET_FAKE(ebus_create);
    RESET_FAKE(ebus_free);
    RESET_FAKE(ebus_queue_size);
    RESET_FAKE(ebus_register_handler);
    RESET_FAKE(ebus_uregister_handler);
    RESET_FAKE(ebus_push);
    RESET_FAKE(ebus_pop);
    RESET_FAKE(ebus_cycle);
}
