#include <vsclib.h>
#include <vscpplib.hpp>
#include <memory>
#include <array>
#include "catch.hpp"

using vsc::vsc_ptr;

static void *test_align(std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space)
{
    void *exp_ptr = ptr, *act_ptr = ptr;
    void *exp_ret, *act_ret;
    size_t exp_space = space, act_space = space;

    exp_ret = std::align(alignment, size, exp_ptr, exp_space);
    act_ret = vsc_align(alignment, size, &act_ptr, &act_space);

    CHECK(exp_ret == act_ret);
    CHECK(exp_ptr == act_ptr);
    CHECK(exp_space == act_space);

    return exp_ret;
}

/*
 * Test for our use case of allocating a block of memory with a header,
 * with both the header and block requiring alignment.
 */
static void test_align_header(size_t header_size, size_t header_align, size_t data_size, size_t data_align)
{
    struct Info {
        void  *p;
        size_t space;
        size_t pad;
        size_t total_size;
    };

    CHECK(VSC_IS_POT(header_align));
    CHECK(VSC_IS_POT(data_align));

    Info stdinfo{};
    stdinfo.p          = reinterpret_cast<void*>(header_size);
    stdinfo.space      = ~size_t(0);
    std::align(data_align, data_size, stdinfo.p, stdinfo.space);
    stdinfo.pad        = ~size_t(0) - stdinfo.space;
    stdinfo.total_size = header_size + data_size + stdinfo.pad;

    Info vscinfo{};
    vscinfo.p          = reinterpret_cast<void*>(header_size);
    vscinfo.space      = ~size_t(0);
    vsc_align(data_align, data_size, &vscinfo.p, &vscinfo.space);
    vscinfo.pad        = ~size_t(0) - vscinfo.space;
    vscinfo.total_size = header_size + data_size + vscinfo.pad;

    CHECK(stdinfo.p          == vscinfo.p);
    CHECK(stdinfo.space      == vscinfo.space);
    CHECK(stdinfo.pad        == vscinfo.pad);
    CHECK(stdinfo.total_size == vscinfo.total_size);

    vsc_ptr<void> block(vsc_aligned_malloc(vscinfo.total_size, header_align));
    if(!block)
        throw std::bad_alloc();

    Info stdinfo2{};
    stdinfo2.p     = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block.get()) + header_size);
    stdinfo2.space = vscinfo.total_size;
    std::align(data_align, data_size, stdinfo2.p, stdinfo2.space);

    Info vscinfo2{};
    vscinfo2.p     = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block.get()) + header_size);
    vscinfo2.space = vscinfo.total_size;
    vsc_align(data_align, data_size, &vscinfo2.p, &vscinfo2.space);

    CHECK(stdinfo2.p     == vscinfo2.p);
    CHECK(stdinfo2.space == vscinfo2.space);

    CHECK(VSC_IS_ALIGNED(block.get(), header_align));
    CHECK(VSC_IS_ALIGNED(vscinfo2.p,  data_align));
}


TEST_CASE("align", "[memory]") {
    void *ret, *p;
    size_t space;

    p     = nullptr;
    space = ~size_t(0);
    ret   = test_align(16, 16, p, space);
    CHECK(p   == nullptr);
    CHECK(ret == nullptr);

    p     = (void*)17;
    space = ~size_t(0);
    ret   = test_align(16, 1234, p, space);
    CHECK(p   == (void*)17);
    CHECK(ret == (void*)32);

    /* Now try our actual use case. */
    test_align_header(4, 32, 4, 64);

    {
        struct TestHeader {
            uint32_t x, y;
            uint16_t z;
        };

        std::array<VscBlockAllocInfo, 4> bai = {{
            /* 1 dummy header */
            {  1, sizeof(TestHeader), alignof(TestHeader) },
            /* 10 uint32_t's */
            { 10, sizeof(uint32_t),   alignof(uint32_t)   },
            /* 3 "float vector 3"'s, with a required alignment of 16 */
            {  3, 12,                 16                   },
            {  1,  1,                 4096                 },
        }};

        std::array<void*, bai.size()> ptrs{};

        vsc_ptr<void> block(vsc_block_alloc(ptrs.data(), bai.data(), bai.size()));
        if(!block)
            throw std::bad_alloc();

        for(size_t i = 0; i < bai.size(); ++i) {
            CHECK(ptrs[i] != NULL);
            if(i > 0)
                CHECK(ptrs[i] > ptrs[i-1]);
            CHECK(VSC_IS_ALIGNED(ptrs[i], bai[i].alignment));
            CHECK((uintptr_t)ptrs[i] % bai[i].alignment == 0);
        }
    }
}
