cmake_minimum_required(VERSION 3.11)

project(vsclib)

option(VSCLIB_BUILD_PARANOID "Enable ASAN/UBSAN" OFF)
option(VSCLIB_ENABLE_TESTS "Enable test applications" ON)

add_subdirectory(vsclib)
add_subdirectory(vscpplib)

if(VSCLIB_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if(VSCLIB_BUILD_PARANOID AND (CMAKE_C_COMPILER_ID STREQUAL "GCC" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
    foreach(target vsclib vscpplib)
        target_compile_options(${target} PRIVATE -Werror -Wextra -pedantic -fsanitize=undefined -fsanitize=address)
    endforeach()

    target_link_options(vsclib PUBLIC -fsanitize=undefined -fsanitize=address)
endif()
