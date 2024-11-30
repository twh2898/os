set(TARGET test_libc)

file(GLOB SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/libc/*.c)

add_library(${TARGET} STATIC ${SOURCE_LIST})
target_link_libraries(${TARGET} libc)
