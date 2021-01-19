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

#include <errno.h>
#include <stdarg.h>
#include "vsclib_i.h"

char *vsc_vasprintfa(const vsc_allocator_t *a, const char *fmt, va_list ap)
{
	va_list ap2;
	va_copy(ap2, ap);
	int nreq = vsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);

	/* Not really EINVAL, but vsnprintf doesn't provide. */
	if(nreq < 0)
		return errno = EINVAL, NULL;

	++nreq;
	char *s = vsci_xalloc(a, (size_t)nreq);
	if(s == NULL)
		return errno = ENOMEM, NULL;

	if(vsnprintf(s, (size_t)nreq, fmt, ap) != nreq - 1)
	{
		vsci_xfree(a, s);
		return errno = EINVAL, NULL;
	}

	return s;
}

char *vsc_vasprintf(const char *fmt, va_list ap)
{
	return vsc_vasprintfa(&vsclib_system_allocator, fmt, ap);
}

char *vsc_asprintfa(const vsc_allocator_t *a, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char *s = vsc_vasprintfa(a, fmt, ap);
	va_end(ap);
	return s;
}

char *vsc_asprintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char *s = vsc_vasprintfa(&vsclib_system_allocator, fmt, ap);
	va_end(ap);
	return s;
}
