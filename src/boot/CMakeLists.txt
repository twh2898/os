
file(GLOB BootsectSource "${${PROJECT_NAME}_SOURCE_DIR}/src/boot/*.asm")

add_custom_target(bootsect
    DEPENDS "${CMAKE_BINARY_DIR}/bootsect.bin"
)

add_custom_command(
    OUTPUT
        "${CMAKE_BINARY_DIR}/bootsect.bin"
    COMMAND
        "${CMAKE_ASM_COMPILER}" -f bin "${${PROJECT_NAME}_SOURCE_DIR}/src/boot/bootsect.asm" -o "${CMAKE_BINARY_DIR}/bootsect.bin"
    DEPENDS
        "${BootsectSource}"
    WORKING_DIRECTORY
        "${${PROJECT_NAME}_SOURCE_DIR}/src"
)

add_library(kernel_entry OBJECT "${${PROJECT_NAME}_SOURCE_DIR}/src/boot/kernel_entry.asm")
target_compile_options(kernel_entry PRIVATE -f elf)
