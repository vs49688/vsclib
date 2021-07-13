project(vsclib_tests)

add_executable(vsclib_tests
        main.cpp
        catch.hpp

        string.cpp
        memory.cpp
        hash.cpp

        wav.cpp
)

target_link_libraries(vsclib_tests vsclib vscpplib)
set_target_properties(vsclib_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MINGW)
    set_target_properties(vsclib_tests PROPERTIES
            LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
endif()
