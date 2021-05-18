#include <vsclib.h>
#include <vscpplib.hpp>
#include <memory>
#include <array>
#include "catch.hpp"

const static char crcinput[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

TEST_CASE("crc32", "[hash]") {
    CHECK(vsc_crc32 (crcinput, sizeof(crcinput)) == 0xcbf43926);
}

TEST_CASE("crc32c", "[hash]") {
    CHECK(vsc_crc32c(crcinput, sizeof(crcinput)) == 0xe3069283);
}
