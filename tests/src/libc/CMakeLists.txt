unit_test(
    TARGET test_libc_circbuff
    TEST_FILES test_circbuff.cpp
    TARGET_FILES libc/src/circbuff.c
    INCLUDE_DIRS libc/include
)
