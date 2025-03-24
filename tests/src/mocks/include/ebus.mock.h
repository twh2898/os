#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ebus.h"
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(int, ebus_create, ebus_t *, size_t);
DECLARE_FAKE_VOID_FUNC(ebus_free, ebus_t *);
DECLARE_FAKE_VALUE_FUNC(int, ebus_queue_size, ebus_t *);
DECLARE_FAKE_VALUE_FUNC(int, ebus_register_handler, ebus_t *, ebus_handler_t *);
DECLARE_FAKE_VOID_FUNC(ebus_uregister_handler, ebus_t *, int);
DECLARE_FAKE_VALUE_FUNC(int, ebus_push, ebus_t *, ebus_event_t *);
DECLARE_FAKE_VALUE_FUNC(int, ebus_pop, ebus_t *, ebus_event_t *);
DECLARE_FAKE_VALUE_FUNC(int, ebus_cycle, ebus_t *);

void reset_ebus_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
