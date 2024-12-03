cmake_minimum_required(VERSION 3.15..3.20)
project(os VERSION 0.1.0 LANGUAGES C)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CROSS_PREFIX "$ENV{HOME}/.local/opt/cross")

set(CMAKE_C_COMPILER "${CROSS_PREFIX}/bin/i386-elf-gcc")
set(CMAKE_C_FLAGS "-g -Werror -ffreestanding")
set(CMAKE_AR "${CROSS_PREFIX}/bin/i386-elf-ar" CACHE FILEPATH "" FORCE)
set(CMAKE_LINKER "${CROSS_PREFIX}/bin/i386-elf-ld")

# ------------------------------------------------------------------------------
# NASM SETUP

# https://stackoverflow.com/a/56434253

enable_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

    # Create a compile option that operates on ASM_NASM files
    # If the target has a property NASM_OBJ_FORMAT, use it, otherwise
    # use the environment variable CMAKE_ASM_NASM_OBJECT_FORMAT
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:ASM_NASM>:-f $<IF:$<BOOL:$<TARGET_PROPERTY:NASM_OBJ_FORMAT>>, \
        $<TARGET_PROPERTY:NASM_OBJ_FORMAT>, ${CMAKE_ASM_NASM_OBJECT_FORMAT}>>"
    )
endif()

add_subdirectory(src)

# ------------------------------------------------------------------------------
# OS IMAGE

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/os-image.bin
    COMMAND cat $<TARGET_OBJECTS:boot> ${CMAKE_BINARY_DIR}/kernel.bin > ${CMAKE_BINARY_DIR}/os-image.bin
    DEPENDS
    $<TARGET_OBJECTS:boot>
    ${CMAKE_BINARY_DIR}/kernel.bin)

add_custom_target(os_image ALL
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    )
add_dependencies(os_image boot kernel kernel_image kernel_image_debug)

add_custom_command(TARGET os_image POST_BUILD COMMAND ${CMAKE_COMMAND} -E
    copy ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------------------------------------------------------------------
# TESTS

# include(CTest)
# enable_testing()

# add_subdirectory(tests)
