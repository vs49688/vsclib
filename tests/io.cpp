#include <vsclib.h>
#include <vscpplib.hpp>
#include "catch.hpp"

TEST_CASE("fopen", "[io]") {
    int r;
    FILE *fp;
    vsc::stdio_ptr _fp;

#if defined(_WIN32)
    r = vsc_fopen("NUL", "rb", &fp);
#else
    r = vsc_fopen("/dev/null", "rb", &fp);
#endif
    _fp.reset(fp);
    REQUIRE(r == 0);
}

TEST_CASE("searchpath", "[io]") {
    int r;
    char *s;
    size_t len;

#if defined(_WIN32)
    r = vsc_searchpath("cmd", &s, &len);
#else
    r = vsc_searchpath("sh", &s, &len);
#endif
    REQUIRE(r == 0);
    REQUIRE(s != nullptr);
    REQUIRE(len > 0);
}