cmake_minimum_required(VERSION 3.15..3.20)
project(os VERSION 0.1.0 LANGUAGES C)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CROSS_PREFIX "$ENV{HOME}/.local/opt/cross")
message("Cross prefix is " ${CROSS_PREFIX})

set(CMAKE_C_COMPILER "${CROSS_PREFIX}/bin/i386-elf-gcc")
set(CMAKE_C_FLAGS "-g -Werror -ffreestanding")

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

# ------------------------------------------------------->-----------------------
# LIBC

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/libc/*.c)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/libc/*.h)

add_library(libc STATIC ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(libc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)

# ------------------------------------------------------------------------------
# DRIVERS

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/drivers/*.c)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/drivers/*.h)

add_library(drivers STATIC ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)

# ------------------------------------------------------------------------------
# CPU

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/cpu/*.c ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/cpu/*.asm)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/cpu/*.h)

add_library(cpu STATIC ${SOURCE_LIST} ${HEADER_LIST})
set_target_properties(cpu PROPERTIES NASM_OBJ_FORMAT elf)
target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)

# ------------------------------------------------------------------------------
# KERNEL

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/*.c)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/*.h)
file(GLOB ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.asm)

add_library(kernel STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)
set_target_properties(kernel PROPERTIES NASM_OBJ_FORMAT elf)

# ------------------------------------------------------------------------------
# UNI

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/**/*.c)
file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/**/*.h)
file(GLOB ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/**/*.asm)

add_library(uni STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(uni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)
set_target_properties(uni PROPERTIES NASM_OBJ_FORMAT elf)

# ------------------------------------------------------------------------------
# BOOT

add_library(boot STATIC ${CMAKE_CURRENT_SOURCE_DIR}/boot/boot.asm)
target_compile_options(boot PRIVATE "-i${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(boot PROPERTIES NASM_OBJ_FORMAT bin)

# ------------------------------------------------------------------------------
# LINK

# # THIS IS COMMENTED OUT BECAUSE LINKING STATIC ARCHIVES DOES NOT WORK
# add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
# COMMAND ${CMAKE_LINKER} -T"${CMAKE_CURRENT_SOURCE_DIR}/link.ld" --oformat binary -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
#     $<TARGET_FILE:kernel> $<TARGET_FILE:libc> $<TARGET_FILE:drivers> $<TARGET_FILE:cpu>
#     -nostdlib -L"${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
# DEPENDS kernel libc drivers cpu)

# THIS REPLACES THE ABOVE WITH SINGLE STATIC ARCHIVE TARGET
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    COMMAND ${CMAKE_LINKER} -T"${CMAKE_CURRENT_SOURCE_DIR}/link.ld" --oformat binary -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
        $<TARGET_FILE:uni>
        -nostdlib -L"${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
    DEPENDS uni)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    COMMAND cat $<TARGET_FILE:boot> ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin > ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    DEPENDS boot ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin)
 
add_custom_target(os_image ALL # echo $<TARGET_FILE:boot> $<TARGET_FILE:kernel> 
    DEPENDS 
    ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    )
# add_dependencies(os_image boot kernel)

# include(CTest)
# enable_testing()

