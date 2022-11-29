#include <ostream>
#include <vscpplib.hpp>

bool operator==(const vsc::Colour32& a, const vsc::Colour32& b) noexcept
{
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

