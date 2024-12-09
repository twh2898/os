set(TARGET kernel)

file(GLOB_RECURSE SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm)

add_library(${TARGET} STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${TARGET} libk libc)
set_target_properties(${TARGET} PROPERTIES NASM_OBJ_FORMAT elf)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.bin
	COMMAND ${CMAKE_LINKER} "-T${CMAKE_SOURCE_DIR}/link.ld" --oformat binary -o "${CMAKE_BINARY_DIR}/kernel.bin" $<TARGET_FILE:${TARGET}> -nostdlib "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
    DEPENDS ${TARGET})

add_custom_target(${TARGET}_image ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/kernel.elf
	COMMAND ${CMAKE_LINKER} "-T${CMAKE_SOURCE_DIR}/link.ld" -o "${CMAKE_BINARY_DIR}/kernel.elf" $<TARGET_FILE:${TARGET}> -nostdlib "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
    DEPENDS ${TARGET})

add_custom_target(${TARGET}_image_debug ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.elf)
