set(TARGET test_libk)

add_executable(${TARGET}
    test_libk.cpp
    ${TEST_TARGET_ROOT}/src/libk/src/sys_call.c
)
target_include_directories(${TARGET} PRIVATE ${TEST_TARGET_ROOT}/src/libk/include)
target_link_libraries(${TARGET} PRIVATE gtest gtest_main)
target_compile_definitions(${TARGET} PUBLIC TEST_USER_OWN_TR1_TUPLE=1 TESTING)

add_test(
    NAME ${TARGET}
    COMMAND $<TARGET_FILE:${TARGET}>
)
