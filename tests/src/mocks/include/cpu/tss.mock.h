#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu/tss.h"
#include "fff.h"

DECLARE_FAKE_VOID_FUNC(init_tss);
DECLARE_FAKE_VALUE_FUNC(tss_entry_t *, tss_get_entry, size_t);
DECLARE_FAKE_VOID_FUNC(tss_set_esp0, uint32_t);

void reset_cpu_tss_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
