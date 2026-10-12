/// Java String.hashCode 语义对齐测试（向量来自 JDK String::hashCode 可复算值）。
#include <gtest/gtest.h>

#include "jstd/string_hash.hpp"

namespace {

using jstd::string_hash::java_string_hash;

TEST(JavaStringHashTest, MatchesJavaLangStringHashCode)
{
    EXPECT_EQ(java_string_hash("hello", 5), 99162322);
    EXPECT_EQ(java_string_hash("", 0), 0);
    // "Az".hashCode() = 65*31 + 122 = 2137。
    EXPECT_EQ(java_string_hash("Az", 2), 2137);
    // 多字节按 UTF-16 码元计：'é' U+00E9 → 单码元 233。
    EXPECT_EQ(java_string_hash("\xC3\xA9", 2), 233);
}

TEST(JavaStringHashTest, SupplementaryCodePointCountsAsSurrogatePair)
{
    // U+1D54F → 高位 0xD835 + 低位 0xDD4F：h = 0xD835*31 + 0xDD4F = 1772474。
    EXPECT_EQ(java_string_hash("\xF0\x9D\x95\x8F", 4), 1772474);
}

}   // namespace
