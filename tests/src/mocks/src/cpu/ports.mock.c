#include "cpu/ports.mock.h"

DEFINE_FAKE_VALUE_FUNC(uint8_t, port_byte_in, uint16_t);
DEFINE_FAKE_VOID_FUNC(port_byte_out, uint16_t, uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t, port_word_in, uint16_t);
DEFINE_FAKE_VOID_FUNC(port_word_out, uint16_t, uint16_t);

void reset_cpu_ports_mock() {
    RESET_FAKE(port_byte_in);
    RESET_FAKE(port_byte_out);
    RESET_FAKE(port_word_in);
    RESET_FAKE(port_word_out);
}
