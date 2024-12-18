
function(unit_test)
    set(oneValueArgs TARGET)
    set(multiValueArgs TEST_FILES TARGET_FILES INCLUDE_DIRS)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "${oneValueArgs}" "${multiValueArgs}")

    set(target ${arg_TARGET})

    set(test_files ${arg_TEST_FILES})
    set(target_files ${arg_TARGET_FILES})
    set(include_dirs ${arg_INCLUDE_DIRS})

    list(TRANSFORM target_files PREPEND ${TEST_TARGET_ROOT}/src/)
    list(TRANSFORM include_dirs PREPEND ${TEST_TARGET_ROOT}/src/)

    add_executable(${target}
        ${test_files}
        ${target_files}
    )
    target_include_directories(${target} PRIVATE ${include_dirs})
    target_link_libraries(${target} PRIVATE gtest gtest_main)
    target_compile_definitions(${target} PUBLIC TEST_USER_OWN_TR1_TUPLE=1 TESTING)

    add_test(
        NAME ${target}
        COMMAND $<TARGET_FILE:${target}>
    )
endfunction()

add_subdirectory(drivers)
add_subdirectory(libc)
add_subdirectory(libk)
