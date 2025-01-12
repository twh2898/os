cmake_minimum_required(VERSION 3.15..3.20)
project(os VERSION 0.1.0 LANGUAGES C CXX)

set(TEST_TARGET_ROOT ${CMAKE_SOURCE_DIR}/..)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_C_STANDARD 17)

# Set the global FFF_GCC_FUNCTION_ATTRIBUTES for config.h
set(FFF_GCC_FUNCTION_ATTRIBUTES "__attribute__((weak))")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(external)

# ------------------------------------------------------------------------------
# TESTS

include(CTest)
enable_testing()

add_subdirectory(src)
