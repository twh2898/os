
add_subdirectory(src)

set(TARGET drivers)
add_library(${TARGET} INTERFACE)
target_link_libraries(${TARGET}
    INTERFACE
    driver_lib
    driver_ata
    driver_ramdisk
    driver_legacy)
