
add_subdirectory(boot)

add_library(interrupt OBJECT "${${PROJECT_NAME}_SOURCE_DIR}/src/cpu/interrupt.asm")
target_compile_options(interrupt PRIVATE -f elf)

set(SOURCE_LIST
    cpu/idt.c
    cpu/isr.c
    cpu/ports.c
    cpu/timer.c
    drivers/keyboard.c
    drivers/vga.c
    kernel/kernel.c
    kernel/term.c
    libc/intern/block.c
    libc/intern/page.c
    libc/mem.c
    libc/stdio.c
    libc/string.c
)
list(TRANSFORM SOURCE_LIST PREPEND "${${PROJECT_NAME}_SOURCE_DIR}/src/")

set(HEADER_LIST
    cpu/idt.h
    cpu/isr.h
    cpu/ports.h
    cpu/timer.h
    drivers/keyboard.h
    drivers/vga.h
    kernel/term.h
    libc/intern/block.h
    libc/intern/page.h
    libc/mem.h
    libc/stdio.h
    libc/string.h
)
list(TRANSFORM HEADER_LIST PREPEND "${${PROJECT_NAME}_SOURCE_DIR}/src/")

add_library(kernel OBJECT ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(kernel PRIVATE "${${PROJECT_NAME}_SOURCE_DIR}/src/")

add_library(kernel.bin)
target_link_libraries(kernel.bin kernel_entry kernel interrupt)
add_dependencies(kernel.bin bootsect)
# target_link_options(kernel.bin PRIVATE -Ttext 0x1000)

add_custom_target(os-image ALL
    DEPENDS "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
)

# add_custom_command(
#     OUTPUT
#         "${CMAKE_BINARY_DIR}/kernel.bin"
#     COMMAND
#         i386-elf-ld -Ttext 0x1000 --oformat binary -o "${CMAKE_BINARY_DIR}/kernel.bin" "${CMAKE_BINARY_DIR}/src/libkernel.bin.a"
#     DEPENDS
#         "${CMAKE_BINARY_DIR}/src/libkernel.bin.a"
# )

add_custom_command(
    OUTPUT
        "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
    COMMAND
        cat "${CMAKE_BINARY_DIR}/bootsect.bin" "${CMAKE_BINARY_DIR}/src/libkernel.bin.a" > "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
    DEPENDS
        "${CMAKE_BINARY_DIR}/kernel.bin"
)
