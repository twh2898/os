set(TARGET boot)

add_library(${TARGET} OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/boot.asm)
target_compile_options(${TARGET} PRIVATE "-i${CMAKE_CURRENT_SOURCE_DIR}/src/")
set_target_properties(${TARGET} PROPERTIES NASM_OBJ_FORMAT bin)
