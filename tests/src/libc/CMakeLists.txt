unit_test(
    TARGET test_libc_circbuff
    TEST_FILES test_circbuff.cpp
    TARGET_FILES libc/src/circbuff.c
    INCLUDE_DIRS libc/include
)

unit_test(
    TARGET test_libc_memory
    TEST_FILES test_memory.cpp
    TARGET_FILES libc/src/memory.c
    INCLUDE_DIRS libc/include libk/include
)

unit_test(
    TARGET test_libc_string
    TEST_FILES test_string.cpp
    TARGET_FILES libc/src/string.c
    INCLUDE_DIRS libc/include libk/include
)
