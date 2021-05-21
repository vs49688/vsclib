#include <vsclib.h>
#include <vscpplib.hpp>
#include <memory>
#include <array>
#include "catch.hpp"

const static char crcinput[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

TEST_CASE("crc32", "[hash]") {
    CHECK(vsc_crc32(nullptr, 0) == 0);
    CHECK(vsc_crc32(crcinput, sizeof(crcinput)) == 0xcbf43926);
}

TEST_CASE("crc32c", "[hash]") {

    uint8_t buf[32];

    CHECK(vsc_crc32c(nullptr, 0) == 0);
    CHECK(vsc_crc32c(crcinput, sizeof(crcinput)) == 0xe3069283);

    /* Run the test vectors from https://datatracker.ietf.org/doc/html/rfc3720#appendix-B.4 */

    /* 32 bytes of zeros */
    for(int i = 0; i < 32; ++i)
        buf[i] = 0;
    CHECK(vsc_crc32c(buf, sizeof(buf)) == 0x8a9136aa);

    /* 32 bytes of ones */
    for(int i = 0; i < 32; ++i)
        buf[i] = ~(uint8_t)0;
    CHECK(vsc_crc32c(buf, sizeof(buf)) == 0x62a8ab43);

    /* 32 bytes of decrementing 1f..00 */
    for(int i = 0; i < 32; ++i)
        buf[i] = (uint8_t)(32 - i - 1);
    CHECK(vsc_crc32c(buf, sizeof(buf)) == 0x113fdb5c);

    /* An iSCSI - SCSI Read (10) Command PDU */
    const static uint8_t iscsi_read[] = {
        0x01, 0xc0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x14, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x14,
        0x00, 0x00, 0x00, 0x18,
        0x28, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    CHECK(vsc_crc32c(iscsi_read, sizeof(iscsi_read)) == 0xd9963a56);
}
