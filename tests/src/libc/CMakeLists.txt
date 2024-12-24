unit_test(
    TARGET test_libc_circbuff
    TEST_FILES test_circbuff.cpp
    TARGET_FILES libc/src/circbuff.c
    INCLUDE_DIRS libc/include
)

unit_test(
    TARGET test_libc_circular_buffer
    TEST_FILES test_circular_buffer.cpp
    TARGET_FILES libc/src/circular_buffer.c
    INCLUDE_DIRS libc/include
)

unit_test(
    TARGET test_libc_memory
    TEST_FILES test_memory.cpp
    TARGET_FILES libc/src/memory.c
    INCLUDE_DIRS libc/include libk/include
)

unit_test(
    TARGET test_libc_process
    TEST_FILES test_process.cpp
    TARGET_FILES libc/src/process.c
    INCLUDE_DIRS libc/include libk/include
)

unit_test(
    TARGET test_libc_string
    TEST_FILES test_string.cpp
    TARGET_FILES libc/src/string.c
    INCLUDE_DIRS libc/include libk/include
)
