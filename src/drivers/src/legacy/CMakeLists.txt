set(TARGET driver_legacy)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc cpu)
