unit_test(
    TARGET test_paging
    TEST_FILES test_paging.cpp
    TARGET_FILES kernel/src/paging.c
)

unit_test(
    TARGET test_process
    TEST_FILES test_process.cpp
    TARGET_FILES kernel/src/proc.c
)
