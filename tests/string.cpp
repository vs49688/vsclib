#include <vsclib.h>
#include <vscpplib.hpp>
#include <string_view>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>
#include "catch.hpp"

using namespace std::string_view_literals;
using vsc::vsc_ptr;

TEST_CASE("strdupr", "[string]") {
    std::string_view original = "****hello, world****"sv;

    vsc_ptr<char> actual(vsc_strdupr(original.data() + 4, original.data() + original.size() - 4));
    REQUIRE(actual);
    REQUIRE(strcmp("hello, world", actual.get()) == 0);
}

TEST_CASE("asprintf", "[string]") {
    vsc_ptr<char> actual(vsc_asprintf("****%s, %s****", "hello", "world"));
    REQUIRE(actual);
    REQUIRE(strcmp("****hello, world****", actual.get()) == 0);
}

static void test_for_each_delim(std::string_view input, const std::vector<std::string_view>& expected) {
    std::vector<std::string_view> actual;
    actual.reserve(expected.size());

    int r = vsc_for_each_delim(input.data(), input.data() + input.size(), ',', [](const char *b, const char *e, void *user){
        std::string_view tok(b, e - b);
        std::vector<std::string_view> *out = reinterpret_cast<std::vector<std::string_view>*>(user);
        out->push_back(tok);
        return 0;
    }, &actual);

    REQUIRE(expected == actual);
    REQUIRE(0 == r);
}

TEST_CASE("for_each_delim, w/o trailing delimiter", "[string]") {
    std::vector<std::string_view> expected = {"a", "b", "c", "d", "", "", "sdf\nfds", "d"};
    test_for_each_delim("a,b,c,d,,,sdf\nfds,d"sv, expected);
}

TEST_CASE("for_each_delim, w/ trailing delimiter", "[string]") {
    std::vector<std::string_view> expected = {"a", "b", "c", "d", "", "", "sdf\nfds", ""};
    test_for_each_delim("a,b,c,d,,,sdf\nfds,"sv, expected);
}

TEST_CASE("for_each_delim, w/ leading delimiter", "[string]") {
    std::vector<std::string_view> expected = {"", "b", "c", "d", "", "", "sdf\nfds", ""};
    test_for_each_delim(",b,c,d,,,sdf\nfds,"sv, expected);
}

static void test_for_each_delim2(std::string_view input, const std::vector<std::string_view>& expected) {
    std::vector<std::string_view> actual;
    actual.reserve(expected.size());

    int r = vsc_for_each_delim(input.data(), input.data() + input.size(), ',', [](const char *b, const char *e, void *user){
        std::string_view tok(b, e - b);
        std::vector<std::string_view> *out = reinterpret_cast<std::vector<std::string_view>*>(user);
        out->push_back(tok);
        return 0;
    }, &actual);

    REQUIRE(expected == actual);
    REQUIRE(0 == r);
}

TEST_CASE("for_each_delim, pass-through return code", "[string]") {
    std::vector<std::string_view> expected = {"a", "b"};

    std::vector<std::string_view> actual;
    actual.reserve(expected.size());

    std::string_view input = "a,b,c,d"sv;

    int r = vsc_for_each_delim(input.data(), input.data() + input.size(), ',', [](const char *b, const char *e, void *user) {
        std::string_view tok(b, e - b);

        if(tok == "c"sv)
            return VSC_ERROR_EOF; /* Any error will do.*/

        std::vector<std::string_view> *out = reinterpret_cast<std::vector<std::string_view>*>(user);
        out->push_back(tok);
        return 0;
    }, &actual);

    REQUIRE(expected == actual);
    REQUIRE(VSC_ERROR_EOF == r);
}

TEST_CASE("getdelim", "[string]") {
    std::ofstream f;
    f.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
    f.open("getdelim-test.txt", std::ios_base::binary);
    f << "a,b,c,d";
    f.close();

    vsc::stdio_ptr fp(fopen("getdelim-test.txt", "rb"));
    REQUIRE(fp);

    char *lineptr = nullptr;
    size_t n = 0;

    vsc_ssize_t nread;

    std::vector<std::string> expected = {"a,", "b,", "c,", "d"};
    std::vector<std::string> actual;
    actual.reserve(expected.size());

    try {
        while((nread = vsc_getdelim(&lineptr, &n, ',', fp.get())) >= 0) {
            actual.emplace_back(lineptr);
        }
    } catch(...) {
        vsc_free(lineptr);
        throw;
    }

    REQUIRE(expected == actual);
    REQUIRE(nread == VSC_ERROR_EOF);
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
TEST_CASE("character conversions", "[string]") {
    size_t len;
    vsc::vsc_ptr<wchar_t> _ws;
    vsc::vsc_ptr<char> _mb;
    wchar_t *ws;
    char *mb;

    ws = vsc_cstrtowstr("abcd", &len, CP_UTF8);
    REQUIRE(ws != nullptr);
    REQUIRE(len == 5);
    _ws.reset(ws);
    CHECK(wcscmp(L"abcd", ws) == 0);

    mb = vsc_wstrtocstr(L"abcd", &len, CP_UTF8);
    REQUIRE(mb != nullptr);
    REQUIRE(len == 5);
    _mb.reset(mb);
    CHECK(strcmp("abcd", mb) == 0);
}
#endif
