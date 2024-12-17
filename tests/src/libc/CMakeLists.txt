set(TARGET test_libc)

add_executable(${TARGET}
    test_circbuff.cpp
    ${TEST_TARGET_ROOT}/src/libc/src/circbuff.c
)
target_include_directories(${TARGET} PRIVATE ${TEST_TARGET_ROOT}/src/libc/include)
target_link_libraries(${TARGET} PRIVATE gtest gtest_main)
target_compile_definitions(${TARGET} PUBLIC TEST_USER_OWN_TR1_TUPLE=1 TESTING)

add_test(
    NAME ${TARGET}
    COMMAND $<TARGET_FILE:${TARGET}>
)
