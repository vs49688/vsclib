cmake_minimum_required(VERSION 3.11)

project(vsclib)

option(VSCLIB_ENABLE_TESTS "Enable test applications" ON)

add_subdirectory(vsclib)
add_subdirectory(vscpplib)

if(VSCLIB_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
