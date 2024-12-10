function(cross_target target)
    file(GLOB_RECURSE SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
    file(GLOB_RECURSE HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm)

    add_library(${target} STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_target_properties(${target} PROPERTIES NASM_OBJ_FORMAT elf)
endfunction()

function(cross_target_binary target)
    get_target_property(TARGET_LINKS ${target} LINK_LIBRARIES)

    set(TARGET_LINK_FILES)
    foreach(item IN ITEMS ${TARGET_LINKS})
        list(APPEND TARGET_LINK_FILES $<TARGET_FILE:${item}>)
    endforeach()

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin
        COMMAND ${CMAKE_LINKER}
        "-T${CMAKE_CURRENT_SOURCE_DIR}/link.ld"
        --oformat binary
        -o "${CMAKE_CURRENT_BINARY_DIR}/${target}.bin"
        $<TARGET_FILE:${target}>
        --start-group
        ${TARGET_LINK_FILES}
        --end-group
        -nostdlib
        "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0"
        -lgcc
        DEPENDS ${target})

    add_custom_target(${target}_image ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin)

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target}.elf
        COMMAND ${CMAKE_LINKER}
        "-T${CMAKE_CURRENT_SOURCE_DIR}/link.ld"
        -o "${CMAKE_CURRENT_BINARY_DIR}/${target}.elf"
        $<TARGET_FILE:${target}>
        --start-group
        ${TARGET_LINK_FILES}
        --end-group
        -nostdlib
        "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0"
        -lgcc
        DEPENDS ${target})

    add_custom_target(${target}_image_debug ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${target}.elf)
endfunction()
