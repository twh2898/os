set(TARGET kernel)

cross_target(${TARGET})
target_link_libraries(${TARGET} utils libc)
cross_target_binary(${TARGET})
