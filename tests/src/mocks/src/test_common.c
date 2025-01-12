#include "test_common.h"

DEFINE_FFF_GLOBALS;

void init_mocks() {
    FFF_RESET_HISTORY();

    reset_cpu_mmu_mock();
    reset_cpu_ports_mock();
    reset_libc_string_mock();
    reset_ram_mock();
}
