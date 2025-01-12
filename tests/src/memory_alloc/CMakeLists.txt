unit_test(
    TARGET test_memory_alloc
    TEST_FILES test_memory_alloc.cpp
    TARGET_FILES memory_alloc/src/memory_alloc.c
)
