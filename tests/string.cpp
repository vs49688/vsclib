#include <vsclib.h>
#include <string_view>
#include <memory>
#include <vector>
#include "catch.hpp"

using namespace std::string_view_literals;

struct vsc_deleter {
    void operator()(void *p) noexcept { vsc_free(p); }
};

template <typename T>
using vsc_ptr = std::unique_ptr<T, vsc_deleter>;

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



