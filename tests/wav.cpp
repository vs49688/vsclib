#include "catch.hpp"
#include <vector>
#include <cmath>
#include <vsclib/io.h>
#include <vsclib/wav.h>
#include <vsclib/hash.h>
#include <vscpplib.hpp>

static void gen_sine_wave(int16_t *buf, size_t size, uint32_t rate, double amplitude, double freq)
{
    const double twopi = 2 * M_PI;
    const double amp   = INT16_MAX * amplitude;
    const double step  = (freq / rate) * twopi;

    double d = 0;
    for(size_t i = 0; i < size; ++i) {
        /* y = A * sin(B(x + C)) + D */
        d      = fmod(d + step, twopi);
        buf[i] = (int16_t)(amp * sin(d));
    }
}

TEST_CASE("wav", "[wav]")
{
    int            r;
    vsc::stdio_ptr f;

    std::vector<int16_t> x(11025 * 5);
    gen_sine_wave(x.data(), x.size(), 11025, 0.8, 261.626);

    f.reset(fopen("middlec-5s.wav", "wb"));
    REQUIRE(f != nullptr);
    r = vsc_wav_write(f.get(), x.data(), x.size(), 11025, 1);
    REQUIRE(r == 0);

    f.reset(fopen("middlec-5s.wav", "rb"));
    REQUIRE(f != nullptr);

    void  *ptr;
    size_t size;
    r = vsc_freadall(&ptr, &size, f.get());
    REQUIRE(r == 0);

    CHECK(110294 == size);
    CHECK(2116742294 == vsc_crc32c(ptr, size));
    vsc_free(ptr);
}
