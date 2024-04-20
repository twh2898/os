
add_library(interrupt OBJECT "${${PROJECT_NAME}_SOURCE_DIR}/src/kernel/src/cpu/interrupt.asm")
target_compile_options(interrupt PRIVATE -f elf)

set(SOURCE_LIST
    cpu/idt.c
    cpu/isr.c
    cpu/ports.c
    cpu/timer.c
    drivers/keyboard.c
    drivers/vga.c
    kernel.c
    term.c
    libc/intern/block.c
    libc/intern/page.c
    libc/mem.c
    libc/stdio.c
    libc/string.c
)
list(TRANSFORM SOURCE_LIST PREPEND "${${PROJECT_NAME}_SOURCE_DIR}/src/kernel/src/")

set(HEADER_LIST
    cpu/idt.h
    cpu/isr.h
    cpu/ports.h
    cpu/timer.h
    drivers/keyboard.h
    drivers/vga.h
    term.h
    libc/intern/block.h
    libc/intern/page.h
    libc/mem.h
    libc/stdio.h
    libc/string.h
)
list(TRANSFORM HEADER_LIST PREPEND "${${PROJECT_NAME}_SOURCE_DIR}/src/kernel/include/")

add_library(kernel OBJECT ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(kernel PRIVATE "${${PROJECT_NAME}_SOURCE_DIR}/src/kernel/include")
