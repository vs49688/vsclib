/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2022 Zane van Iperen (zane@zanevaniperen.com)
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
#include <vsclib/fixed.h>

VscX1616 vsc_float_to_x1616(float val)
{
    return (VscX1616){ .v = (int32_t)(val * VSC_X1616_ONE) };
}

VscX2012 vsc_float_to_x2012(float val)
{
    return (VscX2012){ .v = (int32_t)(val * VSC_X2012_ONE) };
}

VscX0412 vsc_float_to_x0412(float val)
{
    return (VscX0412){ .v = (int16_t)(val * VSC_X0412_ONE) };
}

float vsc_x1616_to_float(VscX1616 val)
{
    return (float)val.v / VSC_X1616_ONE;
}

float vsc_x2012_to_float(VscX2012 val)
{
    return (float)val.v / VSC_X2012_ONE;
}

float vsc_x0412_to_float(VscX0412 val)
{
    return (float)val.v / VSC_X0412_ONE;
}
