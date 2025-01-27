unit_test(
    TARGET test_mmu
    TEST_FILES test_mmu.cpp
    TARGET_FILES cpu/i386/src/mmu.c
)
