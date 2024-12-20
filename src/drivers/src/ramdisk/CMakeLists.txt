set(TARGET ramdisk_driver)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc cpu driver_lib)
