cmake_minimum_required(VERSION 3.15..3.20)
project(os_test VERSION 0.1.0 LANGUAGES C CXX)

set(TEST_TARGET_ROOT ${CMAKE_SOURCE_DIR}/..)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_C_STANDARD 17)

# Set the global FFF_GCC_FUNCTION_ATTRIBUTES for config.h
set(FFF_GCC_FUNCTION_ATTRIBUTES "__attribute__((weak))")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

add_subdirectory(external)

# ------------------------------------------------------------------------------
# TESTS

include(CTest)
enable_testing()

add_subdirectory(src)

if(CMAKE_COMPILER_IS_GNUCXX)
    setup_target_for_coverage_gcovr_html(
        NAME ${PROJECT_NAME}_coverage
        EXECUTABLE ctest
        DEPENDENCIES ${PROJECT_NAME}_all
        BASE_DIRECTORY ../
        EXCLUDE ${PROJECT_SOURCE_DIR}/*
    )
endif()
