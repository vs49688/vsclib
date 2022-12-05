#include <array>
#include "common.hpp"

#define CHECK_CSTRING(a, b) \
    do { \
    if(nullptr == (a) || (nullptr) == (b)) \
        CHECK(a == b); \
    else \
        CHECK(strcmp(a, b) == 0); \
} while(0)

struct hmdel {
    using pointer = VscHashMap*;
    void operator()(pointer p) noexcept { vsc_hashmap_reset(p); }
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
    return vsc_crc32c(key, strlen((const char*)key));
}

static vsc_hash_t hashproc(const void *key)
{
    if(key == nullptr)
        return VSC_INVALID_HASH;

    return vsc_hash(key, strlen((const char*)key));
}

static int compareproc(const void *a, const void *b)
{
    if(a == b)
        return 1;

    if(a == nullptr || b == nullptr)
        return 0;

    return strcmp((const char*)a, (const char*)b) == 0;
}

TEST_CASE("hashmap", "[hashmap]") {
    int r;
    TestAllocator<512> allocator;

    VscHashMap hm;
    hmptr _hm(&hm);


    vsc_hashmap_inita(&hm, hashproc32, compareproc, allocator);

    /* Check initial state. */
    REQUIRE(0           == hm.size);
    REQUIRE(0           == hm.num_buckets);
    REQUIRE(nullptr     == hm.buckets);
    REQUIRE(hashproc32  == hm.hash_proc);
    REQUIRE(compareproc == hm.compare_proc);
    REQUIRE(nullptr     != hm.allocator);
    REQUIRE(VSC_HASHMAP_RESIZE_LOAD_FACTOR == hm.resize_policy);

    /* Disable auto-resizing. */
    hm.resize_policy = VSC_HASHMAP_RESIZE_NONE;

    /* Resize to hold 8 buckets. */
    r = vsc_hashmap_resize(&hm, 8);
    REQUIRE(0 == r);
    REQUIRE(0 == hm.size);
    REQUIRE(8 == hm.num_buckets);

    /* Insert a -> A */
    REQUIRE(vsc_hashmap_insert(&hm, "a", (void*)"A") == 0);

    {
        const char *v1 = (const char*)vsc_hashmap_find(&hm, "a");
        const char *v2 = (const char*)vsc_hashmap_find_by_hash(&hm, vsc_hashmap_hash(&hm, "a"));
        CHECK(v1 == v2);
        CHECK(strcmp(v1, "A") == 0);
    }

    REQUIRE(vsc_hashmap_insert(&hm, "b", (void*)"B") == 0);
    REQUIRE(vsc_hashmap_insert(&hm, "c", (void*)"C") == 0);
    REQUIRE(vsc_hashmap_insert(&hm, "d", (void*)"D") == 0);
    REQUIRE(vsc_hashmap_insert(&hm, "e", (void*)"E") == 0);
    REQUIRE(vsc_hashmap_insert(&hm, "c", (void*)"C++") == 0);
    {
        const char *v1 = (const char*)vsc_hashmap_remove(&hm, "d");
        REQUIRE(nullptr != v1);
        REQUIRE(strcmp("D", v1) == 0);
    }
    {
        const char *v1 = (const char*)vsc_hashmap_remove(&hm, "a");
        REQUIRE(nullptr != v1);
        REQUIRE(strcmp("A", v1) == 0);
    }

    /* We're using a known hash function, ensure our buckets are what we expect. */
    //dumpx(&hm);
    std::array<VscHashMapBucket, 8> expected = {{
        {VSC_INVALID_HASH, nullptr,          nullptr},
        {105567279U,       (const void*)"e", (void*)"E"},
        {VSC_INVALID_HASH, nullptr,          nullptr},
        {VSC_INVALID_HASH, nullptr,          nullptr},
        {3531649220U,      (const void*)"b", (void*)"B"},
        {VSC_INVALID_HASH, nullptr,          nullptr},
        {VSC_INVALID_HASH, nullptr,          nullptr},
        {552285127U,       (const void*)"c", (void*)"C++"},
    }};
    REQUIRE(hm.num_buckets == expected.size());
    for(size_t i = 0; i < expected.size(); ++i) {
        CHECK(expected[i].hash == hm.buckets[i].hash);
        CHECK_CSTRING((const char*)expected[i].key, (const char*)hm.buckets[i].key);
        CHECK_CSTRING((const char*)expected[i].value, (const char*)hm.buckets[i].value);
    }
}


TEST_CASE("hashmap 2", "[hashmap]") {
    VscHashMap hm;
    hmptr _hm(&hm);
    vsc_hashmap_init(&hm, hashproc, compareproc);
    REQUIRE(vsc_hashmap_configure(&hm, 1, 2, 3, 4) == 0);

    char nkeys[1536][6];
    for(size_t i = 0; i < 1536; ++i) {
        sprintf(nkeys[i], "%zu", i);
        REQUIRE(vsc_hashmap_insert(&hm, nkeys[i], nkeys[i]) == 0);
    }

    for(const auto& c : nkeys)
        CHECK_CSTRING(c, (const char*)vsc_hashmap_find(&hm, c));
}

TEST_CASE("hashmap disallow resize", "[hashmap]") {
    VscHashMap hm;
    hmptr _hm(&hm);
    vsc_hashmap_init(&hm, hashproc, compareproc);
    REQUIRE(vsc_hashmap_resize(&hm, 2) == 0);

    /* Disable auto-resizing. */
    hm.resize_policy = VSC_HASHMAP_RESIZE_NONE;

    CHECK(vsc_hashmap_insert(&hm, "a", nullptr) == 0);
    CHECK(vsc_hashmap_insert(&hm, "b", nullptr) == 0);
    CHECK(vsc_hashmap_insert(&hm, "c", nullptr) == VSC_ERROR(ENOSPC));

    hm.resize_policy = VSC_HASHMAP_RESIZE_LOAD_FACTOR;
    CHECK(vsc_hashmap_insert(&hm, "c", nullptr) == 0);
}

TEST_CASE("null keys", "[hashmap]") {
    VscHashMap hm;
    hmptr _hm(&hm);

    vsc_hashmap_init(&hm, hashproc32, compareproc);

    CHECK(vsc_hashmap_insert(&hm, "a", (void*)"a") == 0);
    CHECK(vsc_hashmap_insert(&hm, nullptr, (void*)"NULL") == 0);

    const char *val = (const char*)vsc_hashmap_find(&hm, "a");
    REQUIRE(val != nullptr);
    REQUIRE(strcmp("a", val) == 0);

    val = (const char*)vsc_hashmap_find(&hm, nullptr);
    REQUIRE(val != nullptr);
    REQUIRE(strcmp("NULL", val) == 0);
}

