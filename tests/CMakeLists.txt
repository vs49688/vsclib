project(vsclib_tests)

add_executable(vsclib_tests
        main.cpp
        catch.hpp
)

target_link_libraries(vsclib_tests vsclib)
