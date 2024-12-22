set(TARGET driver_fs_tar)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc cpu driver_lib)
