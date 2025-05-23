set(APPS_BASE_DIR ${CMAKE_BINARY_DIR}/apps)
execute_process(COMMAND mkdir -p ${APPS_BASE_DIR})

macro(add_app target)
    cross_target(${target})
    target_link_libraries(${target} libc)
    cross_target_binary(${target})

    add_custom_command(OUTPUT ${APPS_BASE_DIR}/${target}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin ${APPS_BASE_DIR}/${target}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${target}.bin)

    add_custom_target(${target}_app
        DEPENDS ${APPS_BASE_DIR}/${target})

    list(APPEND APPS_TARGETS ${target} ${target}_app)
    set(APPS_TARGETS ${APPS_TARGETS} PARENT_SCOPE)
endmacro()

add_subdirectory(foo)
add_subdirectory(bar)
add_subdirectory(demo)
add_subdirectory(shell)

message("App targets are ${APPS_TARGETS}")

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/apps.tar
    COMMAND tar -cf ${CMAKE_BINARY_DIR}/apps.tar *
    DEPENDS ${APPS_TARGETS}
    WORKING_DIRECTORY ${APPS_BASE_DIR})

add_custom_target(apps_image ALL
    DEPENDS ${CMAKE_BINARY_DIR}/apps.tar)
