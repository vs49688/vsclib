project(vsclib_tests)

add_executable(vsclib_tests
        main.cpp
        catch.hpp

        string.cpp
)

target_link_libraries(vsclib_tests vsclib)
set_target_properties(vsclib_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
