#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "libc/proc.h"

DECLARE_FAKE_VOID_FUNC(set_next_pid, uint32_t);
DECLARE_FAKE_VOID_FUNC(queue_event, ebus_event_t *);

void reset_libc_proc_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
