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
/** \file vsclib/wav.h */
#ifndef _VSCLIB_WAV_H
#define _VSCLIB_WAV_H

#include <stdio.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Write 16-bit PCM to a WAV file.
 *
 * \param f           The file to write to.
 * \param data        A pointer to the sample data.
 * \param nsamples    The number of samples, per channel.
 * \param sample_rate The audio sample rate.
 * \param nchannels   The number of channels.
 *
 * \return Upon successful completion, vsc_wav_write() returns 0.
 *         Otherwise, returns a negative error value.
 */
int vsc_wav_write(FILE *f, int16_t *data, size_t nsamples, uint32_t sample_rate, uint16_t nchannels);

#if defined(__cplusplus)
}
#endif

#endif /* _VSCLIB_WAV_H */
