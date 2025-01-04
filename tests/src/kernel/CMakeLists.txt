unit_test(
    TARGET test_paging
    TEST_FILES test_paging.cpp
    TARGET_FILES kernel/src/paging.c
    INCLUDE_DIRS kernel/include util/include cpu/include libc/include
)
