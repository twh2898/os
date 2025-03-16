set(TARGET kernel)

cross_target(${TARGET})
target_link_libraries(${TARGET} libk libc cpu io ebus drivers memory_alloc util)
cross_target_binary(${TARGET})
