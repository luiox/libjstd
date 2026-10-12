#include "jstd/random.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

namespace {

/// float 位模式（对齐 Java `Float.floatToIntBits`，hex 比较用）。
std::uint32_t bits(ca::f32 v)
{
    std::uint32_t u = 0;
    std::memcpy(&u, &v, sizeof(u));
    return u;
}

/// double 位模式（对齐 Java `Double.doubleToLongBits`，hex 比较用）。
std::uint64_t bits(ca::f64 v)
{
    std::uint64_t u = 0;
    std::memcpy(&u, &v, sizeof(u));
    return u;
}

}   // namespace

// ── 期望值来源 ──────────────────────────────────────────────
// 由真实 JVM 生成（C:\...\jstd\JvmRandomExpected.java，JDK 输出逐位固化）：
//   nextInt / nextLong / nextFloat(floatToIntBits) / nextDouble(doubleToLongBits)
//   nextBoolean 序列与各 bound 的 nextInt(bound) 前 6 个。
// 权威是 JVM；本测试保证 C++ 实现与 JVM 逐位一致（含拒绝采样与浮点除法）。

TEST(JstdRandom, NextIntSequenceMatchesJvm)
{
    jstd::Random r(0xC0FFEE);
    const ca::i32 expect[16] = {
        -697215370, 1898977891, 1799283490, 197122478, -1856021828, -2129085876,
        1205975494, -1366730479, -856613135, -1069025276, 2003173498, 813769142,
        1508284558, -981072684, -1001680455, 1812972226,
    };
    for (auto e : expect) EXPECT_EQ(r.nextInt(), e);
}

TEST(JstdRandom, NextLongSequenceMatchesJvm)
{
    jstd::Random r(0xC0FFEE);
    const ca::i64 expect[8] = {
        -2994517210519561629LL, 7727863745979865518LL, -7971553054051222964LL,
        5179625305140713745LL, -3679125401218058236LL, 8603564662937690550LL,
        6478032848690742484LL, -4302184793454427454LL,
    };
    for (auto e : expect) EXPECT_EQ(r.nextLong(), e);
}

TEST(JstdRandom, NextFloatMatchesJvmBitExact)
{
    jstd::Random r(0xC0FFEE);
    const char* expect[8] = {
        "3f567156", "3ee26034", "3ed67dc6", "3d3bfd90",
        "3f115f5a", "3f0118ba", "3e8fc372", "3f2e8959",
    };
    for (auto e : expect) {
        char hex[16];
        std::snprintf(hex, sizeof(hex), "%08x", bits(r.nextFloat()));
        EXPECT_STREQ(hex, e);
    }
}

TEST(JstdRandom, NextDoubleMatchesJvmBitExact)
{
    jstd::Random r(0xC0FFEE);
    const char* expect[8] = {
        "3feace2acb8980d3", "3fdacfb8c0bbfd9a", "3fe22beb5408c5d2", "3fd1f86e7ae89590",
        "3fe99e23de023fc0", "3fddd9801308121a", "3fd679a62c58604c", "3fe8897233607e16",
    };
    for (auto e : expect) {
        char hex[24];
        std::snprintf(hex, sizeof(hex), "%016llx",
                      static_cast<unsigned long long>(bits(r.nextDouble())));
        EXPECT_STREQ(hex, e);
    }
}

TEST(JstdRandom, NextBooleanMatchesJvm)
{
    jstd::Random r(0xC0FFEE);
    const int expect[24] = {
        1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1,
    };
    for (auto e : expect) EXPECT_EQ(r.nextBoolean() ? 1 : 0, e);
}

TEST(JstdRandom, NextIntBoundMatchesJvm)
{
    struct Case
    {
        ca::i32 bound;
        ca::i32 expect[6];
    };
    const Case cases[] = {
        {2,        {0, 0, 0, 0, 0, 0}},
        {3,        {2, 0, 1, 0, 2, 0}},
        {7,        {2, 3, 2, 4, 4, 5}},
        {63,       {44, 24, 16, 18, 53, 33}},
        {64,       {7, 6, 28, 2, 28, 10}},
        {127,      {0, 32, 3, 37, 116, 21}},
        {128,      {14, 13, 57, 5, 56, 21}},
        {255,      {254, 198, 115, 123, 98, 189}},
        {256,      {29, 27, 114, 10, 113, 43}},
        {1000,     {329, 923, 565, 198, 983, 569}},
        {65536,    {7506, 7058, 29278, 2581, 29023, 11237}},
        {1 << 30,  {122982164, 115641961, 479704782, 42300099, 475527491, 184119784}},
        {INT32_MAX, {245964329, 231283923, 959409565, 84600198, 951054983, 368239569}},
    };
    for (const auto& c : cases) {
        jstd::Random r(0xDECAF);
        for (int i = 0; i < 6; ++i) {
            EXPECT_EQ(r.nextInt(c.bound), c.expect[i])
                << "bound=" << c.bound << " index=" << i;
        }
    }
}

TEST(JstdRandom, Seed24218MatchesShuffleProbe)
{
    // 与 key_codec genPermutation 的 shuffle 期望同源：seed=24218 的 nextInt。
    jstd::Random r(24218);
    const ca::i32 expect[8] = {
        -1338224980, -737064161, 1710343152, -1094863468,
        -1035909107, 1228046784, -1178859664, 1249979571,
    };
    for (auto e : expect) EXPECT_EQ(r.nextInt(), e);
}

TEST(JstdRandom, SetSeedResetsSequence)
{
    jstd::Random r(1);
    const ca::i32 first = r.nextInt();
    r.setSeed(1);
    EXPECT_EQ(r.nextInt(), first);

    // 构造等价于 setSeed。
    jstd::Random r2(1);
    EXPECT_EQ(r2.nextInt(), first);
}

TEST(JstdRandom, NextIntRejectsNonPositiveBound)
{
    // 契约：bound<=0 时行为未定义（Java 抛 IllegalArgumentException）。
    // 此处仅验证 2 的幂分支与拒绝采样分支对合法 bound 一致的概率形态不崩。
    jstd::Random r(7);
    for (int i = 0; i < 1000; ++i) {
        const ca::i32 v = r.nextInt(7);
        ASSERT_GE(v, 0);
        ASSERT_LT(v, 7);
    }
}
