unit_test(
    TARGET test_vga
    TEST_FILES test_vga.cpp
    TARGET_FILES drivers/src/vga.c
    INCLUDE_DIRS drivers/include cpu/include libc/include
)
