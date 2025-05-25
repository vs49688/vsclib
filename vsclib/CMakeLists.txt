project(vsclib)

include(TestBigEndian)
include(CheckSymbolExists)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)

TEST_BIG_ENDIAN(VSC_ENDIAN_BIG)

check_type_size(max_align_t MAX_ALIGN_T LANGUAGE C)
set(VSC_HAVE_MAX_ALIGN_T ${HAVE_MAX_ALIGN_T})

set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists(getgrent_r "grp.h" VSC_HAVE_GETGRENT_R)

set(CMAKE_REQUIRED_DEFINITIONS)
check_symbol_exists(stpcpy "string.h" VSC_HAVE_STPCPY)
check_symbol_exists(strcpy "string.h" VSC_HAVE_STRCPY)

# MSVC Intrinsics
if(MSVC)
	check_include_files(intrin.h VSC_HAVE_INTRIN_H)

	check_c_source_compiles("
	#include <intrin.h>
	#pragma intrinsic(_BitScanForward)
	int main(void) { unsigned long l; (void)_BitScanForward(&l, 1); return 0; }
	" VSC_HAVE_BITSCANFORWARD)

	check_c_source_compiles("
	#include <intrin.h>
	#pragma intrinsic(_BitScanForward64)
	int main(void) { unsigned long l; (void)_BitScanForward64(&l, 1); return 0; }
	" VSC_HAVE_BITSCANFORWARD64)
endif()

check_type_size("int" VSC_SIZEOF_INT LANGUAGE C)
check_type_size("long" VSC_SIZEOF_LONG LANGUAGE C)
check_type_size("long long" VSC_SIZEOF_LONG_LONG LANGUAGE C)
check_type_size("size_t" VSC_SIZEOF_SIZE_T LANGUAGE C)
check_type_size("int64_t" VSC_SIZEOF_INT64_T LANGUAGE C)

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
		strcpy.c
		stpcpy.c
		strjoin.c
		string.c

		memory.c
		allocator_internal.h

		ctz.c

		sysmem.c
		time.c

		hash.c
		crc32.c
		xxhash.h

		wav.c

		hashmap.c

		error.c

		allocator.cpp

		uuid.c
		include/vsclib/uuiddef.h
		include/vsclib/uuid.h

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set_source_files_properties(allocator.cpp PROPERTIES
			COMPILE_OPTIONS "-fno-exceptions;-fno-rtti"
	)
endif()

set_target_properties(vsclib PROPERTIES
		C_STANDARD 11
		C_STANDARD_REQURED ON
)
target_include_directories(vsclib PUBLIC
		include
		${VSCLIB_GENERATED_INCLUDE_DIR}/include
)

target_compile_definitions(vsclib PUBLIC "$<$<CONFIG:DEBUG>:VSC_DEBUG=1>")

if(MSVC)
	target_compile_definitions(vsclib PRIVATE _CRT_SECURE_NO_WARNINGS=0)
endif()
