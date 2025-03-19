set(TARGET cpu)

cross_target(${TARGET}) # defined in parent CMakeLists.txt
target_link_libraries(${TARGET} libk libc util)
