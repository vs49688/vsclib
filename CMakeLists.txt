cmake_minimum_required(VERSION 3.11)

project(vsclib)

option(VSCLIB_BUILD_PARANOID "Enable ASAN/UBSAN" OFF)
option(VSCLIB_ENABLE_TESTS "Enable test applications" ON)

add_subdirectory(vsclib)
add_subdirectory(vscpplib)

if(VSCLIB_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if(VSCLIB_BUILD_PARANOID)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
        foreach(target vsclib vscpplib)
            target_compile_options(${target} PRIVATE -Wextra -pedantic -fsanitize=undefined -fsanitize=address)
            target_link_options(${target} PUBLIC -fsanitize=undefined -fsanitize=address)
        endforeach()
    endif()
endif()
