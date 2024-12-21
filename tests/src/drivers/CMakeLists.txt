unit_test(
    TARGET test_ata
    TEST_FILES test_ata.cpp
    TARGET_FILES drivers/src/ata/src/ata.c
    INCLUDE_DIRS
    drivers/src/lib/include
    drivers/src/ata/include
    cpu/include
    libc/include
)

unit_test(
    TARGET test_ramdisk
    TEST_FILES test_ramdisk.cpp
    TARGET_FILES drivers/src/ramdisk/src/ramdisk.c
    INCLUDE_DIRS
    drivers/src/lib/include
    drivers/src/ramdisk/include
    cpu/include
    libc/include
)

unit_test(
    TARGET test_vga
    TEST_FILES test_vga.cpp
    TARGET_FILES drivers/src/legacy/src/vga.c
    INCLUDE_DIRS
    drivers/src/legacy/include
    cpu/include
    libc/include
)
