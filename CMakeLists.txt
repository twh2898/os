cmake_minimum_required(VERSION 3.15..3.20)
project(os VERSION 0.1.0 LANGUAGES C)

# set(CMAKE_VERBOSE_MAKEFILE on)

set(CROSS_PREFIX "$ENV{HOME}/.local/opt/cross")
message("Cross prefix is " ${CROSS_PREFIX})

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

# ------------------------------------------------------------------------------
# UNI

file(GLOB_RECURSE SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/src/*.c)
file(GLOB_RECURSE HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include/*.h)
file(GLOB_RECURSE ASM_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.asm)

message("Kernel asm ${ASM_SOURCE_LIST}")

add_library(uni STATIC ${ASM_SOURCE_LIST} ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(uni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include)
set_target_properties(uni PROPERTIES NASM_OBJ_FORMAT elf)

# ------------------------------------------------------------------------------
# BOOT

add_library(boot OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/boot/boot.asm)
target_compile_options(boot PRIVATE "-i${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(boot PROPERTIES NASM_OBJ_FORMAT bin)

# ------------------------------------------------------------------------------
# LINK

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
	COMMAND ${CMAKE_LINKER} "-T${CMAKE_CURRENT_SOURCE_DIR}/link.ld" --oformat binary -o "${CMAKE_CURRENT_BINARY_DIR}/kernel.bin" $<TARGET_FILE:uni> -nostdlib "-L${CROSS_PREFIX}/lib/gcc/i386-elf/12.2.0" -lgcc
    DEPENDS uni)

add_custom_target(kernel ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    COMMAND cat $<TARGET_OBJECTS:boot> ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin > ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    DEPENDS boot ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin)
 
add_custom_target(os_image ALL # echo $<TARGET_FILE:boot> $<TARGET_FILE:kernel> 
    DEPENDS 
    ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin
    ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    )
add_dependencies(os_image boot kernel)

add_custom_command(TARGET os_image POST_BUILD COMMAND ${CMAKE_COMMAND} -E
    copy ${CMAKE_CURRENT_BINARY_DIR}/os-image.bin ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------------------------------------------------------------------
# TESTS

# include(CTest)
# enable_testing()

# # find_package(gtest REQUIRED)

# add_executable(test1 tests/test_main.cpp)
# target_link_libraries(test1 gtest)

# add_test(NAME test1 COMMAND test1)
