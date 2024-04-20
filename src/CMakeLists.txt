
add_subdirectory(boot)
add_subdirectory(kernel)

add_library(kernel.bin)
target_link_libraries(kernel.bin kernel_entry kernel interrupt)
add_dependencies(kernel.bin bootsect)
# target_link_options(kernel.bin PRIVATE -Ttext 0x1000)

add_custom_target(os-image ALL
    DEPENDS
        "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
)

add_custom_command(
    OUTPUT
        "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
    COMMAND
        cat "${CMAKE_BINARY_DIR}/bootsect.bin" "${CMAKE_BINARY_DIR}/src/libkernel.bin.a" > "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
    DEPENDS
        "${CMAKE_BINARY_DIR}/bootsect.bin"
        "${CMAKE_BINARY_DIR}/src/libkernel.bin.a"
)

add_custom_command(TARGET os-image
    POST_BUILD
    COMMAND
    echo "Final image size"
    COMMAND
	du -sh "${${PROJECT_NAME}_SOURCE_DIR}/os-image.bin"
    COMMAND
	echo Remember the limit is 32K in bootsect.asm
    VERBATIM
)
