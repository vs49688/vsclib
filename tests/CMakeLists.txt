project(vsclib_tests)

add_executable(vsclib_tests
        main.cpp
        catch.hpp

        string.cpp
        memory.cpp
        hash.cpp
        hashmap.cpp

        wav.cpp
        time.cpp
        io.cpp
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

    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
    add_subdirectory(mingw-std-threads)
    target_link_libraries(vsclib_tests mingw_stdthreads)
endif()

target_compile_definitions(vsclib_tests PRIVATE "-D_USE_MATH_DEFINES")
