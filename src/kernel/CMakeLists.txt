set(TARGET kernel)

cross_target(${TARGET})
target_link_libraries(${TARGET} libc libk)
cross_target_binary(${TARGET})

# add_custom_target(${TARGET}_image ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin)

# add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.elf
# 	COMMAND ${CMAKE_LINKER} "-T${CMAKE_SOURCE_DIR}/link.ld" -o "${CMAKE_BINARY_DIR}/kernel.elf" $<TARGET_FILE:${TARGET}> -nostdlib "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
#     DEPENDS ${TARGET})

# add_custom_target(${TARGET}_image_debug ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.elf)
