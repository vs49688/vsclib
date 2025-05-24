#include <array>
#include "common.hpp"

#define CHECK_CSTRING(a, b)                    \
    do {                                       \
        if(nullptr == (a) || (nullptr) == (b)) \
            CHECK(a == b);                     \
        else                                   \
            CHECK(strcmp(a, b) == 0);          \
    } while(0)

struct hmdel {
    using pointer = VscHashMap *;
    void operator()(pointer p) noexcept
    {
        vsc_hashmap_free(p);
    }
};
using hmptr = std::unique_ptr<VscHashMap, hmdel>;

#if 0
static void dumpx(const VscHashMap *hm)
{
    printf("%zu buckets:\n", hm->num_buckets);
    for(size_t i = 0; i < hm->num_buckets; ++i) {
        const VscHashMapBucket *bkt = hm->buckets + i;
        printf("  [%2zu] = ", i);
        if(bkt->hash == VSC_INVALID_HASH) {
            printf("/* empty */\n");
            continue;
        }

        printf("index = %zu, (%s, %s)\n", bkt->hash % hm->num_buckets, (const char *)bkt->key, (const char *)bkt->value);
    }
}
#endif

static vsc_hash_t hashproc32(const void *key)
{
    if(key == nullptr)
        return 0;

    /* Abuse crc32c as a hash function because it's fixed-size. */
    /* fprintf(stderr, "%c: %u\n", s[0], vsc_crc32c(s, 1)); */
    return vsc_crc32c(key, strlen((const char *)key));
}

static vsc_hash_t hashproc(const void *key)
{
    if(key == nullptr)
        return VSC_INVALID_HASH;

    return vsc_hash(key, strlen((const char *)key));
}

static int compareproc(const void *a, const void *b)
{
    if(a == b)
        return 1;

    if(a == nullptr || b == nullptr)
        return 0;

    return strcmp((const char *)a, (const char *)b) == 0;
}

TEST_CASE("hashmap", "[hashmap]")
{
    int                r;
    TestAllocator<512> allocator;

    hmptr hm(vsc_hashmap_alloca(hashproc32, compareproc, allocator));

    /* Check initial state. */
    REQUIRE(vsc_hashmap_size(hm.get()) == 0);
    REQUIRE(vsc_hashmap_capacity(hm.get()) == 0);
    REQUIRE(vsc_hashmap_resize_policy(hm.get()) == VSC_HASHMAP_RESIZE_LOAD_FACTOR);

    /* Disable auto-resizing. */
    vsc_hashmap_set_resize_policy(hm.get(), VSC_HASHMAP_RESIZE_NONE);

    /* Resize to hold 8 buckets. */
    r = vsc_hashmap_resize(hm.get(), 8);
    REQUIRE(0 == r);
    REQUIRE(vsc_hashmap_size(hm.get()) == 0);
    REQUIRE(vsc_hashmap_capacity(hm.get()) == 8);

    /* Insert a -> A */
    REQUIRE(vsc_hashmap_insert(hm.get(), "a", (void *)"A") == 0);

    {
        const char *v1 = (const char *)vsc_hashmap_find(hm.get(), "a");
        const char *v2 = (const char *)vsc_hashmap_find_by_hash(hm.get(), vsc_hashmap_hash(hm.get(), "a"));
        CHECK(v1 == v2);
        CHECK(strcmp(v1, "A") == 0);
    }

    REQUIRE(vsc_hashmap_insert(hm.get(), "b", (void *)"B") == 0);
    REQUIRE(vsc_hashmap_insert(hm.get(), "c", (void *)"C") == 0);
    REQUIRE(vsc_hashmap_insert(hm.get(), "d", (void *)"D") == 0);
    REQUIRE(vsc_hashmap_insert(hm.get(), "e", (void *)"E") == 0);
    REQUIRE(vsc_hashmap_insert(hm.get(), "c", (void *)"C++") == 0);
    {
        const char *v1 = (const char *)vsc_hashmap_remove(hm.get(), "d");
        REQUIRE(nullptr != v1);
        REQUIRE(strcmp("D", v1) == 0);
    }
    {
        const char *v1 = (const char *)vsc_hashmap_remove(hm.get(), "a");
        REQUIRE(nullptr != v1);
        REQUIRE(strcmp("A", v1) == 0);
    }

    /* We're using a known hash function, ensure our buckets are what we expect. */
    // dumpx(&hm);
    std::array<VscHashMapBucket, 8> expected = {
        {
         {VSC_INVALID_HASH, nullptr, nullptr},
         {105567279U, (const void *)"e", (void *)"E"},
         {VSC_INVALID_HASH, nullptr, nullptr},
         {VSC_INVALID_HASH, nullptr, nullptr},
         {3531649220U, (const void *)"b", (void *)"B"},
         {VSC_INVALID_HASH, nullptr, nullptr},
         {VSC_INVALID_HASH, nullptr, nullptr},
         {552285127U, (const void *)"c", (void *)"C++"},
         }
    };

    REQUIRE(vsc_hashmap_capacity(hm.get()) == expected.size());
    const VscHashMapBucket *first = vsc_hashmap_first(hm.get());
    for(size_t i = 0; i < expected.size(); ++i) {
        CHECK(expected[i].hash == first[i].hash);
        CHECK_CSTRING((const char *)expected[i].key, (const char *)first[i].key);
        CHECK_CSTRING((const char *)expected[i].value, (const char *)first[i].value);
    }
}

TEST_CASE("hashmap 2", "[hashmap]")
{
    hmptr hm(vsc_hashmap_alloc(hashproc32, compareproc));

    REQUIRE(vsc_hashmap_configure(hm.get(), 1, 2, 3, 4) == 0);

    char nkeys[1536][6];
    for(size_t i = 0; i < 1536; ++i) {
        snprintf(nkeys[i], sizeof(nkeys[i]), "%zu", i);
        REQUIRE(vsc_hashmap_insert(hm.get(), nkeys[i], nkeys[i]) == 0);
    }

    for(const auto& c : nkeys)
        CHECK_CSTRING(c, (const char *)vsc_hashmap_find(hm.get(), c));
}

TEST_CASE("hashmap disallow resize", "[hashmap]")
{
    hmptr hm(vsc_hashmap_alloc(hashproc32, compareproc));

    REQUIRE(vsc_hashmap_resize(hm.get(), 2) == 0);

    /* Disable auto-resizing. */
    vsc_hashmap_set_resize_policy(hm.get(), VSC_HASHMAP_RESIZE_NONE);

    CHECK(vsc_hashmap_insert(hm.get(), "a", nullptr) == 0);
    CHECK(vsc_hashmap_insert(hm.get(), "b", nullptr) == 0);
    CHECK(vsc_hashmap_insert(hm.get(), "c", nullptr) == VSC_ERROR(ENOSPC));

    vsc_hashmap_set_resize_policy(hm.get(), VSC_HASHMAP_RESIZE_LOAD_FACTOR);
    CHECK(vsc_hashmap_insert(hm.get(), "c", nullptr) == 0);
}

TEST_CASE("hashmap check size", "[hashmap]")
{
    hmptr hm(vsc_hashmap_alloc(hashproc32, compareproc));

    REQUIRE(vsc_hashmap_resize(hm.get(), 2) == 0);

    CHECK(vsc_hashmap_insert(hm.get(), "a", nullptr) == 0);
    CHECK(vsc_hashmap_insert(hm.get(), "b", nullptr) == 0);
    CHECK(vsc_hashmap_size(hm.get()) == 2);

    /* Replace the value for "b", the size should be the same. */
    CHECK(vsc_hashmap_insert(hm.get(), "b", (void *)1) == 0);
    CHECK(vsc_hashmap_size(hm.get()) == 2);
}

TEST_CASE("null keys", "[hashmap]")
{
    hmptr hm(vsc_hashmap_alloc(hashproc32, compareproc));

    CHECK(vsc_hashmap_insert(hm.get(), "a", (void *)"a") == 0);
    CHECK(vsc_hashmap_insert(hm.get(), nullptr, (void *)"NULL") == 0);

    const char *val = (const char *)vsc_hashmap_find(hm.get(), "a");
    REQUIRE(val != nullptr);
    REQUIRE(strcmp("a", val) == 0);

    val = (const char *)vsc_hashmap_find(hm.get(), nullptr);
    REQUIRE(val != nullptr);
    REQUIRE(strcmp("NULL", val) == 0);
}
