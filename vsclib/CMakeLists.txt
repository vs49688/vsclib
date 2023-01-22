project(vsclib)

include(TestBigEndian)
include(CheckSymbolExists)

TEST_BIG_ENDIAN(VSC_ENDIAN_BIG)

check_type_size(max_align_t MAX_ALIGN_T LANGUAGE C)
set(VSC_HAVE_MAX_ALIGN_T ${HAVE_MAX_ALIGN_T})

set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists(getgrent_r "grp.h" HAVE_GETGRENT_R)
set(VSC_HAVE_GETGRENT_R ${HAVE_GETGRENT_R})

set(CMAKE_REQUIRED_DEFINITIONS)
check_symbol_exists(stpcpy "string.h" HAVE_STPCPY)
set(VSC_HAVE_STPCPY ${HAVE_STPCPY})

if(MINGW OR WIN32)
	set(OS_FILES
			searchpath_win32.c
			cstrtowstr_win32.c
			wstrtocstr_win32.c
			util_win32.c
			util_win32.h
	)
else()
	set(OS_FILES
			searchpath.c
	)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(VSC_DEBUG 1)
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
		colour.c
		enum_groups.c
		strdup.c
		strdupr.c
		stpcpy.c

		memory.c
		sysmem.c
		time.c

		hash.c
		crc32.c
		xxhash.h

		wav.c

		hashmap.c

		error.c

		mingw-aligned-malloc.c
		mingw-aligned-malloc.h
		allocator.c

		${OS_FILES}

		vsclib_config.h.in
		${VSCLIB_GENERATED_INCLUDE_DIR}/include/vsclib_config.h
		include/vsclib.h
		include/vsclib/platform.h

		include/vsclib/assert.h
		include/vsclib/types.h
		include/vsclib/macros.h
		include/vsclib/error.h

		include/vsclib/memdef.h
		include/vsclib/mem.h

		include/vsclib/iodef.h
		include/vsclib/io.h

		include/vsclib/stringdef.h
		include/vsclib/string.h

		include/vsclib/hashdef.h
		include/vsclib/hash.h

		include/vsclib/wav.h

		include/vsclib/hashmapdef.h
		include/vsclib/hashmap.h

		include/vsclib/timedef.h
		include/vsclib/time.h

		include/vsclib/enum_groups.h

		include/vsclib/colourdef.h
		include/vsclib/colour.h
)

set_target_properties(vsclib PROPERTIES
		C_STANDARD 11
		C_STANDARD_REQURED ON
)
target_include_directories(vsclib PUBLIC
		include
		${VSCLIB_GENERATED_INCLUDE_DIR}/include
)

if(MSVC)
	target_compile_definitions(vsclib PRIVATE _CRT_SECURE_NO_WARNINGS=0)
endif()
