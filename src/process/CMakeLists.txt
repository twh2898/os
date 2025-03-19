set(TARGET process)

cross_target(${TARGET})
target_link_libraries(${TARGET} cpu libk libc ebus)
