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

        void *header;
        void *uints;
        void *floats;
        void *zeroarray;
        void *bigalign;

        std::array<VscBlockAllocInfo, 5> bai = {{
            /* 1 dummy header */
            {  1, sizeof(TestHeader), alignof(TestHeader), &header    },
            /* 10 uint32_t's */
            { 10, sizeof(uint32_t),   alignof(uint32_t),   &uints     },
            /* 3 "float vector 3"'s, with a required alignment of 16 */
            {  3, 12,                 16,                  &floats    },
            {  0,  4,                 16,                  &zeroarray },
            {  1,  1,                 4096,                &bigalign  },
        }};

        std::array<void*, bai.size()> ptrs{};

        int r = vsc_block_alloc(ptrs.data(), bai.data(), bai.size(), 0);
        REQUIRE(r == 0);

        vsc_ptr<void> block(ptrs[0]);
        if(!block)
            throw std::bad_alloc();

        CHECK(header    == ptrs[0]);
        CHECK(uints     == ptrs[1]);
        CHECK(floats    == ptrs[2]);
        CHECK(zeroarray == ptrs[3]);
        CHECK(bigalign  == ptrs[4]);
        CHECK(zeroarray == nullptr);

        for(size_t i = 0; i < bai.size(); ++i) {
            const VscBlockAllocInfo *b = bai.data() + i;

            if(b->element_size * b->count != 0)
                CHECK(ptrs[i] != nullptr);

            /* All alignments bets are off when we're empty. */
            if(b->count == 0 || b->element_size == 0)
                continue;

            if(i > 0)
                CHECK(ptrs[i] > ptrs[i-1]);
            CHECK(VSC_IS_ALIGNED(ptrs[i], b->alignment));
            CHECK((uintptr_t)ptrs[i] % b->alignment == 0);
        }
    }
}

template<size_t N, size_t A>
class TestAllocator : private VscAllocator {
    static_assert(VSC_IS_POT(A));

public:
    TestAllocator() :
        VscAllocator{alloc_stub, free_stub, size_stub, VSC_ALIGNOF(vsc_max_align_t), this},
        buf_{},
        offset_(0)
    {
        uintptr_t p;
        for(p = (uintptr_t)buf_; p <= (uintptr_t)buf_ + N; p += A) {
            if((p % A) == 0 && (p % (A << 1)) != 0)
                break;
        }

        if(p >= (uintptr_t)buf_ + N)
            throw std::runtime_error("alignment error");

        this->offset_ = p - (uintptr_t)buf_;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    operator const VscAllocator*() const noexcept {
        return this;
    }
#pragma clang diagnostic pop

private:
    int alloc_cb(void **ptr, size_t size, size_t alignment, VscAllocFlags flags) noexcept {
        void *p = (void*)((uintptr_t)buf_ + this->offset_);
        size_t space = N - this->offset_;

        if(flags & VSC_ALLOC_REALLOC)
            return VSC_ERROR(ENOTSUP);

        void *pp = vsc_align(alignment, size, &p, &space);
        if(pp == nullptr) {
            if(flags & VSC_ALLOC_NOFAIL)
                std::terminate();

            return VSC_ERROR(EINVAL);
        }

        if(flags & VSC_ALLOC_ZERO)
            memset(pp, 0, size);

        *ptr = pp;
        return 0;
    }

    void free_cb(void *p) noexcept {
        (void)p;
    }

    size_t size_cb(void *p) noexcept {
        (void)p;
        return N;
    }

    static int alloc_stub(void **ptr, size_t size, size_t alignment, VscAllocFlags flags, void *user) noexcept {
        return reinterpret_cast<TestAllocator*>(user)->alloc_cb(ptr, size, alignment, flags);
    }

    static void free_stub(void *p, void *user) noexcept {
        return reinterpret_cast<TestAllocator*>(user)->free_cb(p);
    }

    static size_t size_stub(void *p, void *user) noexcept {
        return reinterpret_cast<TestAllocator*>(user)->size_cb(p);
    }

    alignas(A) uint8_t buf_[N];
    size_t offset_;
};

/*
 * Ensure vsc_block_xalloc() gives the closest alignments, e.g.
 * Given 2 blocks, with (size, align):
 *   b1 = (  64,  8)
 *   b2 = (1024, 16)
 * Require that b2 is at the next closest 16-byte alignment after b1.
 *
 * There's two cases, the allocator returns an 8, or 16-byte aligned
 * address for b1 - this test covers both of them.
 *
 * if b1 is  8-byte aligned, b2 == b1 + 64 + 8
 * if b2 is 16-byte aligned, b2 == b1 + 64
 */
TEST_CASE("block allocate test next closest alignment", "[memory]") {
    TestAllocator<2048, 8> t8;
    TestAllocator<2048, 16> t16;

    VscBlockAllocInfo bai[2] = {
        {1,   64,  8, nullptr},
        {1, 1024, 16, nullptr},
    };

    void *ptrs8[2];
    int r = vsc_block_xalloc(t8, ptrs8, bai, 2, 0);
    REQUIRE(r == 0);

    uintptr_t rptr8[2] = {
        reinterpret_cast<uintptr_t>(ptrs8[0]),
        reinterpret_cast<uintptr_t>(ptrs8[1]),
    };

    CHECK(VSC_IS_ALIGNED(ptrs8[0], 8));
    CHECK(!VSC_IS_ALIGNED(ptrs8[0], 16)); /* Our allocator should enforce this. */
    CHECK(VSC_IS_ALIGNED(ptrs8[1], 16));

    CHECK(rptr8[1] > rptr8[0]);
    CHECK(VSC_IS_ALIGNED(rptr8[0] + bai[0].element_size + 8, 16));
    CHECK(rptr8[1] == rptr8[0] + bai[0].element_size + 8);

    void *ptrs16[2];
    r = vsc_block_xalloc(t16, ptrs16, bai, 2, 0);
    REQUIRE(r == 0);

    uintptr_t rptr16[2] = {
        reinterpret_cast<uintptr_t>(ptrs16[0]),
        reinterpret_cast<uintptr_t>(ptrs16[1]),
    };
    CHECK(VSC_IS_ALIGNED(ptrs16[0], 16));
    CHECK(VSC_IS_ALIGNED(ptrs16[1], 16));
    CHECK(rptr16[1] > rptr16[0]);
    CHECK(VSC_IS_ALIGNED(rptr16[0] + bai[0].element_size, 16));
    CHECK(rptr16[1] == rptr16[0] + bai[0].element_size);
}

TEST_CASE("zero", "[memory]") {
    uint8_t *p;
    vsc_ptr<uint8_t> _p;

    p = (uint8_t*)vsc_calloc(1024, 1);
    REQUIRE(p != nullptr);
    _p.reset(p);

    for(size_t i = 0; i < 1024; ++i) {
         CHECK(p[i] == 0);
         p[i] = UINT8_MAX;
    }

    int ret = vsc_xalloc_ex(&vsclib_system_allocator, (void**)&p, 2048, VSC_ALLOC_REALLOC | VSC_ALLOC_ZERO, 0);
    REQUIRE(ret == 0);
    REQUIRE(p != nullptr);
    (void)_p.release();
    _p.reset(p);

    for(size_t i = 0; i < 2048; ++i) {
        if(i < 1024)
            CHECK(p[i] == UINT8_MAX);
        else
            CHECK(p[i] == 0);
    }
}

TEST_CASE("realloc", "[memory]") {
    uint8_t *p;
    vsc_ptr<void> _p;

    p = (uint8_t*)vsc_malloc(10);
    REQUIRE(p != nullptr);
    _p.reset(p);

    for(size_t i = 0; i < 10; ++i)
        p[i] = 0xFE;

    uint8_t *p2 = (uint8_t*)vsc_realloc(p, 100);
    REQUIRE(p2 != nullptr);
    (void)_p.release();
    _p.reset(p2);

    for(size_t i = 0; i < 10; ++i)
        CHECK(p2[i] == 0xFE);
}
