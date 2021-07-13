/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2021 Zane van Iperen (zane@zanevaniperen.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _VSCLIB_IO_H
#define _VSCLIB_IO_H

#include <string.h>

#include "platform.h"
#include "iodef.h"
#include "memdef.h"

#if defined(__cplusplus)
extern "C" {
#endif

FILE *vsc_fopen(const char *pathname, const char *mode);

/**
 * @brief fopen(), but correctly handles UTF-8 pathnames on Windows.
 *
 * Memory is allocated in a linear-compatible fashion.
 */
FILE *vsc_fopena(const char *pathname, const char *mode, const VscAllocator *a);

int       vsc_fileno(FILE *stream);
int       vsc_open(const char *pathname, int flags);
int       vsc_dup2(int oldfd, int newfd);
int       vsc_close(int fd);
vsc_off_t vsc_ftello(FILE *stream);
int       vsc_fseeko(FILE *stream, vsc_off_t offset, int whence);
int       vsc_freadall(void **ptr, size_t *size, FILE *f);
int       vsc_freadalla(void **ptr, size_t *size, FILE *f, const VscAllocator *a);
int       vsc_freadall_ex(void **ptr, size_t *size, FILE *f, VscFreadallInitProc init_proc,
                          VscFreadallChunkProc chunk_proc, void *user);

/**
 * @brief Read an entire stream into memory.
 *
 * @param ptr        The pointer to receive the allocated buffer.
 * @param size       A pointer to receive the size of the file.
 * @param f          The stream to read.
 * @param init_proc  A initialisation callback that may used to inspect and modify stream parameters
 *                   before reading commences. Only the `file_size` and `blk_size` fields of `VscFreadallState`
 *                   may be modified.
 * @param chunk_proc A callback used to report progress. This is invoked after each internal read.
 * @param user       A user-provided pointer to be passed to `init_proc` and `chunk_proc`
 * @param a          The allocator to use.
 *
 * @return Upon successful completion vsc_freadalla() returns 0, or -1 if an error
 *         occurred and errno is set to indicate the error. If the failure is caused by a negative
 *         return from `init_proc` or `chunk_proc`, `errno` will be `ECANCELED`.
 */
int vsc_freadalla_ex(void **ptr, size_t *size, FILE *f, VscFreadallInitProc init_proc,
                     VscFreadallChunkProc chunk_proc, void *user, const VscAllocator *a);

/**
 * @brief Nullify a stream, that is open the platform's bitbucket and dup2() it
 * into @p f.
 *
 * @param f The stream to nullify.
 * @return Upon successful completion vsc_fnullify() returns the file descriptor
 * placed in @par f. Otherwise, -1 is returned and errno is set to indicate the error.
 */
int vsc_fnullify(FILE *f);

char *vsc_searchpath(const char *f, size_t *len);
char *vsc_searchpatha(const char *f, size_t *len, const VscAllocator *a);

int   vsc_chdir(const char *path);
int   vsc_chdira(const char *path, const VscAllocator *a);

#if (_MSC_VER >= 1400)
static inline uint16_t vsc_swap_uint16(uint16_t x) { return _byteswap_ushort(x); }
static inline uint32_t vsc_swap_uint32(uint32_t x) { return _byteswap_ulong(x); }
static inline uint64_t vsc_swap_uint64(uint64_t x) { return _byteswap_uint64(x); }
#elif defined(__GNUC__)
static inline uint16_t vsc_swap_uint16(uint16_t x) { return __builtin_bswap16(x); }
static inline uint32_t vsc_swap_uint32(uint32_t x) { return __builtin_bswap32(x); }
static inline uint64_t vsc_swap_uint64(uint64_t x) { return __builtin_bswap64(x); }
#else
static inline uint16_t vsc_swap_uint16(uint16_t x)
{
    return ((x & 0x00FFU) >> 0U) << 8U |
           ((x & 0xFF00U) >> 8U) << 0U ;
}

static inline uint32_t vsc_swap_uint32(uint32_t x)
{
    return ((x & 0x000000FFU) >>  0U) << 24U |
           ((x & 0x0000FF00U) >>  8U) << 16U |
           ((x & 0x00FF0000U) >> 16U) <<  8U |
           ((x & 0xFF000000U) >> 24U) <<  0U ;
}

static inline uint64_t vsc_swap_uint64(uint64_t x)
{
    return ((x & 0x00000000000000FFULL) >>  0U) << 56U |
           ((x & 0x000000000000FF00ULL) >>  8U) << 48U |
           ((x & 0x0000000000FF0000ULL) >> 16U) << 40U |
           ((x & 0x00000000FF000000ULL) >> 24U) << 32U |
           ((x & 0x000000FF00000000ULL) >> 32U) << 24U |
           ((x & 0x0000FF0000000000ULL) >> 40U) << 16U |
           ((x & 0x00FF000000000000ULL) >> 48U) <<  8U |
           ((x & 0xFF00000000000000ULL) >> 56U) <<  0U ;
}
#endif

static inline int16_t vsc_swap_int16(int16_t x) { return (int16_t)vsc_swap_uint16((uint16_t)x); }
static inline int32_t vsc_swap_int32(int32_t x) { return (int32_t)vsc_swap_uint32((uint32_t)x); }
static inline int64_t vsc_swap_int64(int64_t x) { return (int64_t)vsc_swap_uint64((uint64_t)x); }

#define VSCLIB_DECLARE_IO_X2Y2X(type, c1, c2, proc) \
    static inline type vsc_##c1##_to_##c2(type x) { return (type)proc(x); } \
    static inline type vsc_##c2##_to_##c1(type x) { return (type)proc(x); }

#define VSCLIB_DECLARE_XXX(_native, _foreign, bits) \
    VSCLIB_DECLARE_IO_X2Y2X( int##bits##_t, _native ##e ##bits, native, ) \
    VSCLIB_DECLARE_IO_X2Y2X(uint##bits##_t, _native ##eu##bits, native, ) \
    VSCLIB_DECLARE_IO_X2Y2X( int##bits##_t, _foreign##e ##bits, native, vsc_swap##_int##bits) \
    VSCLIB_DECLARE_IO_X2Y2X(uint##bits##_t, _foreign##eu##bits, native, vsc_swap##_uint##bits)

#define VSCLIB_DECLARE_IO_CONVERTERS(native, foreign) \
    VSCLIB_DECLARE_XXX(native, foreign, 16) \
    VSCLIB_DECLARE_XXX(native, foreign, 32) \
    VSCLIB_DECLARE_XXX(native, foreign, 64)

#if defined(VSC_ENDIAN_BIG)
VSCLIB_DECLARE_IO_CONVERTERS(b, l)
#else
VSCLIB_DECLARE_IO_CONVERTERS(l, b)
#endif

#undef VSCLIB_DECLARE_IO_CONVERTERS
#undef VSCLIB_DECLARE_IO_X2Y2X

#define VSCLIB_DECLARE_READWRITE(bits) \
    static inline void vsc_write_uint##bits(void *p, uint##bits##_t v) \
    { memcpy(p, &v, sizeof(v)); } \
    \
    static inline void vsc_write_int##bits(void *p, int##bits##_t v) \
    { memcpy(p, &v, sizeof(v)); } \
    static inline uint##bits##_t vsc_read_uint##bits(const void *p) \
    { uint##bits##_t v; memcpy(&v, p, sizeof(v)); return v; } \
    \
    static inline int##bits##_t vsc_read_int##bits(const void *p) \
    { int##bits##_t v; memcpy(&v, p, sizeof(v)); return v; }

#define VSCLIB_DECLARE_READWRITE_E(endian, bits) \
    static inline void vsc_write_##endian##eu##bits(void *p, uint##bits##_t val) \
    { vsc_write_uint##bits(p, vsc_native_to_##endian##eu##bits(val)); } \
    \
    static inline void vsc_write_##endian## e##bits(void *p, int##bits##_t val) \
    { vsc_write_int ##bits(p, vsc_native_to_##endian##e ##bits(val)); } \
    \
    static inline uint##bits##_t vsc_read_##endian##eu##bits(const void *p) \
    { return vsc_##endian##eu##bits##_to_native(vsc_read_uint##bits(p)); } \
    \
    static inline int##bits##_t vsc_read_##endian##e ##bits(const void *p) \
    { return vsc_##endian##e##bits##_to_native(vsc_read_int##bits(p)); }

VSCLIB_DECLARE_READWRITE(8);
VSCLIB_DECLARE_READWRITE(16);
VSCLIB_DECLARE_READWRITE(32);
VSCLIB_DECLARE_READWRITE(64);

VSCLIB_DECLARE_READWRITE_E(l, 16);
VSCLIB_DECLARE_READWRITE_E(b, 16);
VSCLIB_DECLARE_READWRITE_E(l, 32);
VSCLIB_DECLARE_READWRITE_E(b, 32);
VSCLIB_DECLARE_READWRITE_E(l, 64);
VSCLIB_DECLARE_READWRITE_E(b, 64);

/* stdio variants of vsc_{read,write} */
#define VSCLIB_DECLARE_FREADWRITE(bits) \
    static inline size_t vsc_fwrite_uint##bits(FILE *f, uint##bits##_t val) \
    { return fwrite(&val, sizeof(val), 1, f); } \
    \
    static inline size_t vsc_fwrite_int##bits (FILE *f,  int##bits##_t val) \
    { return fwrite(&val, sizeof(val), 1, f); } \
    \
    static inline uint##bits##_t vsc_fread_uint##bits(FILE *f) \
    { uint##bits##_t val; fread(&val, sizeof(val), 1, f); return val; } \
    \
    static inline int##bits##_t vsc_fread_int##bits (FILE *f) \
    {  int##bits##_t val; fread(&val, sizeof(val), 1, f); return val; }

#define VSCLIB_DECLARE_FREADWRITE_E(endian, bits) \
    static inline size_t vsc_fwrite_##endian##eu##bits(FILE *f, uint##bits##_t val) \
    { return vsc_fwrite_uint##bits(f, vsc_native_to_##endian##eu##bits(val)); } \
    \
    static inline size_t vsc_fwrite_##endian## e##bits(FILE *f,  int##bits##_t val) \
    { return vsc_fwrite_int ##bits(f, vsc_native_to_##endian##e ##bits(val)); } \
    \
    static inline uint##bits##_t vsc_fread_##endian##eu##bits(FILE *f) \
    { return vsc_##endian##eu##bits##_to_native(vsc_fread_uint##bits(f)); } \
    \
    static inline int##bits##_t vsc_fread_##endian##e ##bits(FILE *f) \
    { return vsc_##endian##e##bits##_to_native(vsc_fread_int##bits(f)); }

VSCLIB_DECLARE_FREADWRITE(8);
VSCLIB_DECLARE_FREADWRITE(16);
VSCLIB_DECLARE_FREADWRITE(32);
VSCLIB_DECLARE_FREADWRITE(64);

VSCLIB_DECLARE_FREADWRITE_E(l, 16);
VSCLIB_DECLARE_FREADWRITE_E(b, 16);
VSCLIB_DECLARE_FREADWRITE_E(l, 32);
VSCLIB_DECLARE_FREADWRITE_E(b, 32);
VSCLIB_DECLARE_FREADWRITE_E(l, 64);
VSCLIB_DECLARE_FREADWRITE_E(b, 64);

#undef VSCLIB_DECLARE_FREADWRITE
#undef VSCLIB_DECLARE_FREADWRITE_E

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_IO_H */
