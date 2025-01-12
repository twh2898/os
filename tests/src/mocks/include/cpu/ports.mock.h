#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu/ports.h"
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(uint8_t, port_byte_in, uint16_t);
DECLARE_FAKE_VOID_FUNC(port_byte_out, uint16_t, uint8_t);
DECLARE_FAKE_VALUE_FUNC(uint16_t, port_word_in, uint16_t);
DECLARE_FAKE_VOID_FUNC(port_word_out, uint16_t, uint16_t);

void reset_cpu_ports_mock(void);

#ifdef __cplusplus
} // extern "C"
#endif
