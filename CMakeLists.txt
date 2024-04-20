cmake_minimum_required(VERSION 3.15...3.20)

project(osv2
    VERSION 0.1
    DESCRIPTION "os in cmake"
    LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_COMPILER "i386-elf-gcc")
set(CMAKE_ASM_COMPILER "nasm")
set(CMAKE_LINKER "i386-elf-ld")

set(CMAKE_C_FLAGS "-Werror -ffreestanding")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_MINSIZEREL "")

set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "")
set(CMAKE_ASM_FLAGS_MINSIZEREL "")

set(CMAKE_VERBOSE_MAKEFILE on)
set(BUILD_TOOL_OPTIONS "-v -d explain")

# set(CMAKE_C_COMPILER_AR "<CMAKE_AR> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_LINKER> <FLAGS> -o <OBJECT> <SOURCE>")
set(CMAKE_C_AR_EXECUTABLE "<CMAKE_C_LINKER> <FLAGS> -o <OBJECT> <SOURCE>")

# set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_LINKER> -Ttext 0x1000 --oformat binary -Map out.map <LINK_FLAGS> -o <TARGET> <OBJECTS>")
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_LINKER> -Map out.map -T ${${PROJECT_NAME}_SOURCE_DIR}/link.ld <LINK_FLAGS> -o <TARGET> <OBJECTS>")

# Set the default build type if non is specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' since none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)

    # Set possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Only if this is the top level project (not included with add_subdirectory)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Use -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available for top level projects. It calls enable_testing
    # which must be in the main CMakeLists.
    include(CTest)
endif()

add_subdirectory(src)

# Add tests
#if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
