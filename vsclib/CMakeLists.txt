project(vsclib)

include(TestBigEndian)

TEST_BIG_ENDIAN(VSC_ENDIAN_BIG)

if(MINGW OR WIN32)
	set(OS_FILES searchpath_win32.c)
else()
	set(OS_FILES searchpath.c)
endif()

set(VSCLIB_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}" )
configure_file("${PROJECT_SOURCE_DIR}/vsclib_config.h.in"
			   "${VSCLIB_GENERATED_INCLUDE_DIR}/include/vsclib_config.h")

add_library(vsclib STATIC
		freadall.c
		fnullify.c
		vsprintfa.c
		getline.c
		fopena.c
		stdio.c
		for_each.c
		chdir.c
		enum_groups.c
		strdupr.c
		memory.c
		${OS_FILES}

		vsclib_i.h
		vsclib_config.h.in
		${VSCLIB_GENERATED_INCLUDE_DIR}/include/vsclib_config.h
		include/vsclib.h

		include/vsclib/memdef.h
		include/vsclib/mem.h

		include/vsclib/iodef.h
		include/vsclib/io.h
)

set_target_properties(vsclib PROPERTIES
		CMAKE_C_STANDARD 99
		CMAKE_C_STANDARD_REQURED ON
)
target_include_directories(vsclib PUBLIC
		include
		${VSCLIB_GENERATED_INCLUDE_DIR}/include
)

if(MSVC)
	target_compile_definitions(vsclib PRIVATE _CRT_SECURE_NO_WARNINGS=0)
endif()
