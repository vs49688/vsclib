#if !defined(_WIN32)
#undef _GNU_SOURCE
#define _XOPEN_SOURCE 600

#include <string.h>

int vsc_test_strerror_r(int errnum, char *buf, size_t buflen)
{
    return strerror_r(errnum, buf, buflen);
}
#endif
