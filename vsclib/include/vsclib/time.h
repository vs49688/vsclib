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
/** \file vsclib/time.h */
#ifndef _VSCLIB_TIME_H
#define _VSCLIB_TIME_H

#include "timedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the number of nanoseconds after an arbitrary, but fixed point in time.
 *
 * \remark This is a monotonic counter - the value returned by successive calls will
 *         never decrease, but the same value may be returned multiple times depending
 *         on system conditions.
 * \remark This does NOT represent the system/wall-clock time, and as such is only
 *         suitable for calculating periodic differences.
 * \remark vsc_counter_ns() has no failure condition, but may call abort() if retrieving
 *         system counters fails. If this happens, please fix your system.
 */
vsc_counter_t vsc_counter_ns(void);

#ifdef __cplusplus
}
#endif

#endif /* _VSCLIB_TIME_H */
