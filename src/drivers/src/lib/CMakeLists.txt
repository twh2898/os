set(TARGET driver_lib)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc cpu)
