add_subdirectory(libc)

set(APPS_BASE_DIR ${CMAKE_BINARY_DIR}/apps)

macro(add_app target)
    file(GLOB_RECURSE SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
    file(GLOB_RECURSE HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm)

    add_library(${target} STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
    target_link_libraries(${target} PUBLIC libc)
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_target_properties(${target} PROPERTIES NASM_OBJ_FORMAT elf)

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin
        COMMAND ${CMAKE_LINKER} "-T${CMAKE_CURRENT_SOURCE_DIR}/link.ld" --oformat binary -o "${CMAKE_CURRENT_BINARY_DIR}/${target}.bin" $<TARGET_FILE:${target}> -nostdlib "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
        DEPENDS ${target})

    add_custom_command(OUTPUT ${APPS_BASE_DIR}/${target}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin ${APPS_BASE_DIR}/${target}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin)

    add_custom_target(${target}_image
        DEPENDS ${APPS_BASE_DIR}/${target})
endmacro()

add_subdirectory(foo)
add_subdirectory(bar)

set(APPS_TARGETS foo bar)

execute_process(COMMAND mkdir -p ${APPS_BASE_DIR})

set(APPS_TARGETS_FILES ${APPS_TARGETS})
list(TRANSFORM APPS_TARGETS_FILES PREPEND ${APPS_BASE_DIR}/)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/apps.tar
    COMMAND tar -cf ${CMAKE_BINARY_DIR}/apps.tar ${APPS_TARGETS}
    DEPENDS ${APPS_TARGETS_FILES}
    WORKING_DIRECTORY ${APPS_BASE_DIR})

add_custom_target(apps_image ALL
    DEPENDS ${CMAKE_BINARY_DIR}/apps.tar)
