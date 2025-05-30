#include "common.hpp"
#include <vsclib/resource.h>
#include <vscpplib.hpp>

TEST_CASE("basic resource tests", "[resource]")
{
    vsc::res_ptr<void> root;

    void *pp = vsc_res_allocate(nullptr, 0, 0);
    root.reset(pp);

    vsc_res_strdup(pp, "child res 0");
    vsc_res_strdup(pp, "child res 1");
    vsc_res_strdup(pp, "child res 2");
    vsc_res_strdup(pp, "child res 3");
    vsc_res_strdup(pp, "child res 4");
    vsc_res_strdup(pp, "child res 5");
    vsc_res_strdup(pp, "child res 6");
    vsc_res_strdup(pp, "child res 7");
    vsc_res_strdup(pp, "child res 8");
    vsc_res_strdup(pp, "child res 9");

    size_t enum_count = 0;

    vsc_res_enum_children(
        pp,
        [](void *, void *user) {
            *reinterpret_cast<size_t *>(user) += 1;
            return 0;
        },
        &enum_count);

    CHECK(enum_count == 10);

    size_t size = vsc_res_size_total(pp);
    CHECK(size == 120);
}

TEST_CASE("vsc::res_allocate()'d resource invokes C++ destructor", "[resource][cpp]")
{
    struct Test {
        bool *called;

        explicit Test(bool *called) noexcept : called(called)
        {
        }

        ~Test() noexcept
        {
            *called = true;
        }
    };

    bool called = false;

    vsc::res_ptr<Test> t = vsc::res_allocate<Test>(nullptr, &called);
    t.reset();
    CHECK(called == true);
}
