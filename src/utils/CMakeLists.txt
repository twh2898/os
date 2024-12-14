set(TARGET utils)

add_library(${TARGET} INTERFACE)
target_link_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
