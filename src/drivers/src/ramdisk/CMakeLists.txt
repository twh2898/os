set(TARGET driver_ramdisk)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc driver_lib)
