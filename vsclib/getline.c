/* getline.c
 *
 * getdelim(), getline() - read a delimited record from stream, ersatz implementation
 *
 * For more details, see: http://pubs.opengroup.org/onlinepubs/9699919799/functions/getline.html
 *
 * 2021-01-19:
 * - Initial dump from https://github.com/ivanrad/getline,
 *   revision 94a4d0a6d6dec5ab4a4822c5ce2276dd89d0258d
 * - Tweak to use vsci_* functions
 */

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <vsclib/error.h>
#include <vsclib/types.h>
#include <vsclib/mem.h>

#if defined(_MSC_VER)
#if defined(_WIN64)
#define SSIZE_MAX _I64_MAX
#else
#define SSIZE_MAX LONG_MAX
#endif
#endif

vsc_ssize_t vsc_getdelima(char **lineptr, size_t *n, int delim, FILE *stream, const VscAllocator *a)
{
    char  *cur_pos, *new_lineptr;
    size_t new_lineptr_len;
    int    c;

    if(lineptr == NULL || n == NULL || stream == NULL)
        return VSC_ERROR(EINVAL);

    if(*lineptr == NULL) {
        *n = 128; /* init len */
        if((*lineptr = vsc_xalloc(a, *n)) == NULL)
            return VSC_ERROR(ENOMEM);
    }

    cur_pos = *lineptr;
    for(;;) {
        c = getc(stream);

        if(ferror(stream) || (c == EOF && cur_pos == *lineptr))
            return VSC_ERROR_EOF;

        if(c == EOF)
            break;

        if((*lineptr + *n - cur_pos) < 2) {
            if(SSIZE_MAX / 2 < *n) {
#ifdef EOVERFLOW
                return VSC_ERROR(EOVERFLOW);
#else
                return VSC_ERROR(ERANGE); /* no EOVERFLOW defined */
#endif
            }
            new_lineptr_len = *n * 2;

            if((new_lineptr = vsc_xrealloc(a, *lineptr, new_lineptr_len)) == NULL)
                return VSC_ERROR(ENOMEM);

            cur_pos  = new_lineptr + (cur_pos - *lineptr);
            *lineptr = new_lineptr;
            *n       = new_lineptr_len;
        }

        *cur_pos++ = (char)c;

        if(c == delim)
            break;
    }

    *cur_pos = '\0';
    return (vsc_ssize_t)(cur_pos - *lineptr);
}

vsc_ssize_t vsc_getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    return vsc_getdelima(lineptr, n, delim, stream, vsclib_system_allocator);
}

vsc_ssize_t vsc_getlinea(char **lineptr, size_t *n, FILE *stream, const VscAllocator *a)
{
    return vsc_getdelima(lineptr, n, '\n', stream, a);
}

vsc_ssize_t vsc_getline(char **lineptr, size_t *n, FILE *stream)
{
    return vsc_getlinea(lineptr, n, stream, vsclib_system_allocator);
}
