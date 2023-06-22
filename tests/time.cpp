#include <chrono>
#include <thread>
#include <vsclib/time.h>
#include "catch.hpp"

TEST_CASE("counter", "")
{
    using namespace std::chrono_literals;

    vsc_counter_t before = vsc_counter_ns();
    std::this_thread::sleep_for(1s);
    vsc_counter_t after = vsc_counter_ns();

    /* Check for roughly 1 second. */
    CHECK(after - before < 1200000000);
}
