#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <vsclib/error.h>
#include <vsclib/io.h>

#define WAV_HEADER_BUF_SIZE 44

#define FOURCC_RIFF         VSC_FOURCC('R', 'I', 'F', 'F')
#define FOURCC_WAVE         VSC_FOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt_         VSC_FOURCC('f', 'm', 't', ' ')
#define FOURCC_data         VSC_FOURCC('d', 'a', 't', 'a')

int vsc_wav_write(FILE *f, int16_t *data, size_t nsamples, uint32_t sample_rate, uint16_t nchannels)
{
    uint8_t        buf[WAV_HEADER_BUF_SIZE];
    const uint16_t block_align = nchannels * sizeof(int16_t);
    const uint32_t data_size   = (uint32_t)(block_align * nsamples);

    if(f == NULL || data == NULL || sample_rate == 0 || (nchannels != 1 && nchannels != 2))
        return VSC_ERROR(EINVAL);

    if(nsamples >= UINT32_MAX / block_align)
        return VSC_ERROR(ERANGE);

    if(data_size > UINT32_MAX - 36)
        return VSC_ERROR(ERANGE);

    vsc_write_leu32(buf + 0, FOURCC_RIFF);
    vsc_write_leu32(buf + 4, 36 + data_size);
    vsc_write_leu32(buf + 8, FOURCC_WAVE);

    /* "fmt " sub-chunk */
    vsc_write_leu32(buf + 12, FOURCC_fmt_);
    vsc_write_leu32(buf + 16, 16);
    vsc_write_leu16(buf + 20, 1);                         /* format, 1 = pcm */
    vsc_write_leu16(buf + 22, nchannels);                 /* num channels    */
    vsc_write_leu32(buf + 24, sample_rate);               /* sample rate     */
    vsc_write_leu32(buf + 28, sample_rate * block_align); /* byte rate       */
    vsc_write_leu16(buf + 32, block_align);               /* block align     */
    vsc_write_leu16(buf + 34, 16);                        /* bits per sample */

    /* "data" sub-chunk */
    vsc_write_leu32(buf + 36, FOURCC_data);
    vsc_write_leu32(buf + 40, data_size);

    if(fwrite(buf, WAV_HEADER_BUF_SIZE, 1, f) != 1)
        return VSC_ERROR(EIO);

        /* NB: This big-endian path is slow, inefficient, and UNTESTED. */
#if VSC_ENDIAN_BIG
    for(size_t i = 0; i < nsamples * nchannels; ++i) {
        int16_t smp = vsc_native_to_be16(data[i]);
        if(fwrite(&smp, sizeof(smp), 1, f) != 1)
            return VSC_ERROR(EIO);
    }
#else
    if(fwrite(data, data_size, 1, f) != 1)
        return VSC_ERROR(EIO);
#endif

    return 0;
}
