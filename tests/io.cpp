#include <vsclib.h>
#include <vscpplib.hpp>
#include <string_view>
#include <sstream>
#include "catch.hpp"

TEST_CASE("fopen", "[io]")
{
    int            r;
    FILE          *fp = nullptr;
    vsc::stdio_ptr _fp;

    r = vsc_fopen(VSC_DEVNULL, "rb", &fp);
    _fp.reset(fp);
    REQUIRE(r == 0);
}

TEST_CASE("searchpath", "[io]")
{
    int    r;
    char  *s;
    size_t len;

#if defined(_WIN32)
    r = vsc_searchpath("cmd", &s, &len);
#else
    r = vsc_searchpath("sh", &s, &len);
#endif
    REQUIRE(r == 0);
    REQUIRE(s != nullptr);
    REQUIRE(len > 0);
    vsc_free(s);
}

#if !defined(_WIN32)

extern "C" int vsc_test_strerror_r(int errnum, char *buf, size_t buflen);

TEST_CASE("fperror", "[io]")
{
    vsc::stdio_ptr _fp;
    FILE          *fp;

    char buf[1024] = {0};
    fp             = fmemopen(buf, sizeof(buf) - 1, "wb");
    _fp.reset(fp);
    REQUIRE(fp != nullptr);

    vsc_fperror(fp, VSC_ERROR_EOF, "something %d", 10);
    vsc_fperror(fp, VSC_ERROR(ENOMEM), "something %s", "dead");
    vsc_fperror(fp, VSC_MKERR('L', 'E', 'L'), "something %s", "funny");
    vsc_fperror(fp, VSC_MKERR(0xFE, 0xEE, 'L'), "something %s", "sensitive");
    fputc('\0', fp);
    _fp.reset();

    char membuf[1024] = {0};
    vsc_test_strerror_r(ENOMEM, membuf, sizeof(membuf));

    std::stringstream ss;
    ss << "something 10: End of file\n"
       << "something dead: " << std::string_view(membuf) << "\n"
       << "something funny: Unknown error ' LEL'\n"
       << "something sensitive: Unknown error 0x20feee4c\n";
    REQUIRE(std::string_view(buf) == ss.str());
}

#endif
