#ifndef COMMON_HPP
#define COMMON_HPP

#include <vector>
#include <stack>
#include <memory>
#include <vsclib.h>
#include <vscpplib.hpp>
#include "catch.hpp"

template<size_t N, size_t A = VSC_ALIGNOF(vsc_max_align_t)>
class TestAllocator : private VscAllocator {
    static_assert(VSC_IS_POT(A));
    static_assert(N % A == 0);

public:
    TestAllocator() :
        VscAllocator{alloc_stub, free_stub, size_stub, VSC_ALIGNOF(vsc_max_align_t), this},
        buf_{},
        offset_(0)
    {}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    operator const VscAllocator*() const noexcept {
        return this;
    }
#pragma clang diagnostic pop

private:
    int alloc_cb(void **ptr, size_t size, size_t alignment, VscAllocFlags flags) noexcept {
        if(this->offset_ >= N + size)
            return VSC_ERROR(ENOMEM);

        void *p = (void*)((uintptr_t)buf_ + this->offset_);

        size_t space = N - this->offset_;

        if(flags & VSC_ALLOC_REALLOC) {
            REQUIRE(*ptr != nullptr);
            return VSC_ERROR(ENOTSUP);
        }

        void *pp = vsc_align(alignment, size, &p, &space);
        if(pp == nullptr) {
            if(flags & VSC_ALLOC_NOFAIL)
                std::terminate();

            return VSC_ERROR(EINVAL);
        }

        if(flags & VSC_ALLOC_ZERO)
            memset(pp, 0, size);

        *ptr = pp;

        this->offset_ = (size_t)VSC_ALIGN_UP(this->offset_ + size, A);
        try {
            this->allocations.push(pp);
        } catch(...) {
            std::terminate();
        }

        return 0;
    }

    void free_cb(void *p) noexcept {
        CHECK(p == this->allocations.top());
        this->allocations.pop();
        this->offset_ = (uintptr_t)p - (uintptr_t)this->buf_;
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
    std::stack<void*, std::vector<void*>> allocations;
};

#endif /* COMMON_HPP */
