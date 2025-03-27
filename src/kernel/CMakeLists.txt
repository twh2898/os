set(TARGET kernel)

cross_target(${TARGET})
target_link_libraries(${TARGET} libk libc cpu io ebus drivers memory_alloc process util)
cross_target_binary(${TARGET})
