cmake_minimum_required(VERSION 3.15..3.20)
project(os VERSION 0.1.0 LANGUAGES C CXX)

# set(CMAKE_VERBOSE_MAKEFILE on)

# find_package(gtest REQUIRED)

file(GLOB_RECURSE MOCK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mock/*.c ${CMAKE_CURRENT_SOURCE_DIR}/mock/*.h)

add_compile_definitions(TESTING)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

function(mock_package target root_path)
    file(GLOB_RECURSE SOURCE_LIST ${root_path}/src/*.c)
    file(GLOB_RECURSE HEADER_LIST ${root_path}/include/*.h)

    add_library(${target} STATIC ${SOURCE_LIST} ${HEADER_LIST} ${MOCK_SOURCES})
    target_include_directories(${target} PUBLIC ${root_path}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mock)
    set_target_properties(${target} PROPERTIES NASM_OBJ_FORMAT elf)

    file(GLOB TEST_SOURCE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/${target}/*.c)

    foreach(test_src IN ITEMS ${TEST_SOURCE_LIST})
        get_filename_component(test_target ${test_src} NAME_WLE)

        add_executable(${test_target} ${test_src})
        target_link_libraries(${test_target} ${target})

        add_test(NAME ${test_target} COMMAND ${test_target})
    endforeach()
endfunction()

mock_package(libc ${CMAKE_SOURCE_DIR}/../src/libc)

# ------------------------------------------------------------------------------
# TESTS

include(CTest)
enable_testing()
