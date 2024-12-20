set(TARGET drivers)

add_subdirectory(src)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc cpu)
target_link_libraries(${TARGET} ramdisk_driver)
