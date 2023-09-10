#include <string_view>
#include <string>
#include <vsclib/error.h>
#include <vsclib/uuid.h>
#include <vector>
#include "catch.hpp"

TEST_CASE("uuid", "")
{
    SECTION("nil")
    {
        char buf[VSC_UUID_STRING_SIZE];

        VscUUID uuid = vsc_uuid_nil();
        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL);
        CHECK(buf == std::string_view("00000000-0000-0000-0000-000000000000"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER);
        CHECK(buf == std::string_view("00000000-0000-0000-0000-000000000000"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_MS);
        CHECK(buf == std::string_view("{00000000-0000-0000-0000-000000000000}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_MS);
        CHECK(buf == std::string_view("{00000000-0000-0000-0000-000000000000}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("00000000000000000000000000000000"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("00000000000000000000000000000000"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_MS | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("{00000000000000000000000000000000}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_MS | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("{00000000000000000000000000000000}"));
    }

    SECTION("max")
    {
        char buf[VSC_UUID_STRING_SIZE];

        VscUUID uuid = vsc_uuid_max();
        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL);
        CHECK(buf == std::string_view("ffffffff-ffff-ffff-ffff-ffffffffffff"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER);
        CHECK(buf == std::string_view("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_MS);
        CHECK(buf == std::string_view("{ffffffff-ffff-ffff-ffff-ffffffffffff}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_MS);
        CHECK(buf == std::string_view("{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("ffffffffffffffffffffffffffffffff"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_NORMAL | VSC_UUID_FORMAT_MS | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("{ffffffffffffffffffffffffffffffff}"));

        vsc_uuid_format(buf, &uuid, VSC_UUID_FORMAT_UPPER | VSC_UUID_FORMAT_MS | VSC_UUID_FORMAT_COMPACT);
        CHECK(buf == std::string_view("{FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}"));
    }

    SECTION("parse")
    {
        std::string_view         expected = "8580bb1c-eb80-4b92-9f39-91124e353018";
        std::vector<std::string> cases    = {
            "8580bb1c-eb80-4b92-9f39-91124e353018",   "8580BB1C-EB80-4B92-9F39-91124E353018",
            "{8580bb1c-eb80-4b92-9f39-91124e353018}", "{8580BB1C-EB80-4B92-9F39-91124E353018}",
            "8580bb1ceb804b929f3991124e353018",       "8580BB1CEB804B929F3991124E353018",
            "{8580bb1ceb804b929f3991124e353018}",     "{8580BB1CEB804B929F3991124E353018}",
        };

        for(const std::string& v : cases) {
            VscUUID uuuu = vsc_uuid_nil();
            int     r;

            r = vsc_uuid_parse(&uuuu, v.c_str());
            CHECK(r == 0);

            char buf[VSC_UUID_STRING_SIZE];
            vsc_uuid_format(buf, &uuuu, VSC_UUID_FORMAT_NORMAL);

            CHECK(buf == expected);
        }

        {
            VscUUID uuuu;
            int     r;

            r = vsc_uuid_parse(&uuuu, "00000000+0000+0000+0000+000000000000");
            CHECK(r == VSC_ERROR(EINVAL));

            r = vsc_uuid_parse(&uuuu, "00000000+0000-0000-0000-000000000000");
            CHECK(r == VSC_ERROR(EINVAL));
        }
    }
}
