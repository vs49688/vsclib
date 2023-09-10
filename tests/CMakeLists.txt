project(vsclib_tests)

add_executable(vsclib_tests
        main.cpp
        catch.hpp
        common.hpp

        string.cpp
        memory.cpp
        hash.cpp
        hashmap.cpp

        wav.cpp
        time.cpp
        io.cpp
        colour.cpp

        uuid.cpp

        strerror.c
)

target_link_libraries(vsclib_tests vsclib vscpplib)
set_target_properties(vsclib_tests PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON

        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MINGW)
    set_target_properties(vsclib_tests PROPERTIES
            LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
endif()

target_compile_definitions(vsclib_tests PRIVATE "-D_USE_MATH_DEFINES")
