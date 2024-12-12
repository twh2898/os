set(TARGET kernel)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc libk)
cross_target_binary(${TARGET})
