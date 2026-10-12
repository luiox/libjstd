#include "jstd/arith.hpp"

#include <gtest/gtest.h>

#include <climits>
#include <cstdint>

namespace arith = jstd::arith;

// ── 溢出回绕（Java 语义：C++ 有符号溢出为 UB，此处必须回绕）──────────

TEST(JstdArith, AddWraps)
{
    EXPECT_EQ(arith::add(INT32_MAX, 1), INT32_MIN);
    EXPECT_EQ(arith::add(-1, 1), 0);
    EXPECT_EQ(arith::add(INT32_MIN, INT32_MIN), 0);
    EXPECT_EQ(arith::add(1, 2), 3);
}

TEST(JstdArith, SubWraps)
{
    EXPECT_EQ(arith::sub(INT32_MIN, 1), INT32_MAX);
    EXPECT_EQ(arith::sub(0, INT32_MAX), INT32_MIN + 1);
    EXPECT_EQ(arith::sub(10, 4), 6);
}

TEST(JstdArith, MulWraps)
{
    EXPECT_EQ(arith::mul(0x40000000, 2), INT32_MIN);
    EXPECT_EQ(arith::mul(INT32_MIN, -1), INT32_MIN);
    EXPECT_EQ(arith::mul(100000, 100000), 1410065408);   // 10^10 mod 2^32（符号化）
    EXPECT_EQ(arith::mul(6, 7), 42);
}

// ── 除法截断 + 取模符号（JVM 语义）──────────────────────────────

TEST(JstdArith, DivTruncatesTowardZero)
{
    EXPECT_EQ(arith::div(7, 2), 3);
    EXPECT_EQ(arith::div(-7, 2), -3);
    EXPECT_EQ(arith::div(7, -2), -3);
    EXPECT_EQ(arith::div(-7, -2), 3);
    EXPECT_EQ(arith::div(INT32_MIN, -1), INT32_MIN);   // 溢出回绕
}

TEST(JstdArith, RemFollowsDividendSign)
{
    EXPECT_EQ(arith::rem(7, 3), 1);
    EXPECT_EQ(arith::rem(-7, 3), -1);
    EXPECT_EQ(arith::rem(7, -3), 1);
    EXPECT_EQ(arith::rem(-7, -3), -1);
    EXPECT_EQ(arith::rem(INT32_MIN, -1), 0);
    EXPECT_EQ(arith::rem(10, 3), 1);
}

// ── 取负 / abs ────────────────────────────────────────────────

TEST(JstdArith, NegWraps)
{
    EXPECT_EQ(arith::neg(5), -5);
    EXPECT_EQ(arith::neg(INT32_MIN), INT32_MIN);   // 回绕
    EXPECT_EQ(arith::neg(INT32_MAX), INT32_MIN + 1);
}

TEST(JstdArith, AbsJavaSemantics)
{
    EXPECT_EQ(arith::abs(5), 5);
    EXPECT_EQ(arith::abs(-5), 5);
    EXPECT_EQ(arith::abs(INT32_MIN), INT32_MIN);   // Math.abs 不回绕
}

// ── 移位（掩码取模 + 算术/逻辑右移）───────────────────────────────

TEST(JstdArith, ShlMasksShiftCount)
{
    EXPECT_EQ(arith::shl(1, 31), INT32_MIN);
    EXPECT_EQ(arith::shl(-1, 1), -2);
    EXPECT_EQ(arith::shl(1, 33), 2);               // 33 & 31 == 1
}

TEST(JstdArith, ShrIsArithmetic)
{
    EXPECT_EQ(arith::shr(-8, 1), -4);
    EXPECT_EQ(arith::shr(INT32_MIN, 31), -1);
    EXPECT_EQ(arith::shr(8, 1), 4);
}

TEST(JstdArith, UshrIsLogical)
{
    EXPECT_EQ(arith::ushr(-1, 1), INT32_MAX);
    EXPECT_EQ(arith::ushr(INT32_MIN, 1), 0x40000000);
    EXPECT_EQ(arith::ushr(-1, 0), -1);
}

// ── 窄化转换（低位位模式）────────────────────────────────────────

TEST(JstdArith, I2bTakesLowByteSigned)
{
    EXPECT_EQ(arith::i2b(0x7F), 127);
    EXPECT_EQ(arith::i2b(0x80), -128);
    EXPECT_EQ(arith::i2b(0x100), 0);
    EXPECT_EQ(arith::i2b(-129), 127);
}

TEST(JstdArith, I2cTakesLow16Unsigned)
{
    EXPECT_EQ(arith::i2c(-1), 65535);
    EXPECT_EQ(arith::i2c(0x10001), 1);
    EXPECT_EQ(arith::i2c(0x7FFF), 32767);
    EXPECT_EQ(arith::i2c(0x8000), 32768);
}

TEST(JstdArith, I2sTakesLow16Signed)
{
    EXPECT_EQ(arith::i2s(0x8000), -32768);
    EXPECT_EQ(arith::i2s(-1), -1);
    EXPECT_EQ(arith::i2s(0x10000), 0);
}

TEST(JstdArith, L2iTakesLow32)
{
    EXPECT_EQ(arith::l2i(0x100000001LL), 1);
    EXPECT_EQ(arith::l2i(INT64_MIN), 0);
    EXPECT_EQ(arith::l2i(-1LL), -1);
    EXPECT_EQ(arith::l2i(0x7FFFFFFFLL), INT32_MAX);
}

// ── int64 语义 ───────────────────────────────────────────────

TEST(JstdArith, I64WrapSemantics)
{
    EXPECT_EQ(arith::add(INT64_MAX, 1LL), INT64_MIN);
    EXPECT_EQ(arith::sub(INT64_MIN, 1LL), INT64_MAX);
    EXPECT_EQ(arith::div(INT64_MIN, -1LL), INT64_MIN);
    EXPECT_EQ(arith::rem(INT64_MIN, -1LL), 0);
    EXPECT_EQ(arith::abs(INT64_MIN), INT64_MIN);
    EXPECT_EQ(arith::neg(INT64_MIN), INT64_MIN);
}

TEST(JstdArith, I64ShiftsMaskBy63)
{
    // 首参显式 ca::i64：LP64 下 i64=long，`1LL`（long long）会同时命中
    // i32/i64 两个重载且转换平级，产生二义性（Windows i64=long long 不复现）。
    EXPECT_EQ(arith::shl(ca::i64{1}, 63), INT64_MIN);
    EXPECT_EQ(arith::ushr(ca::i64{-1}, 1), INT64_MAX);
    EXPECT_EQ(arith::shr(ca::i64{-1}, 63), -1);
    EXPECT_EQ(arith::shl(ca::i64{1}, 65), 2);   // 65 & 63 == 1
    EXPECT_EQ(arith::ushr(INT64_MIN, 63), 1);
}

// ── Java 与 C++ 行为对照（对拍点，非权威——权威为 JVM fixture）─────

TEST(JstdArith, MixedRoundTripAgainstKnownValues)
{
    // 常规值不被回绕/截断破坏。
    EXPECT_EQ(arith::add(3, 4), 7);
    EXPECT_EQ(arith::sub(4, 9), -5);
    EXPECT_EQ(arith::mul(-3, 5), -15);
    EXPECT_EQ(arith::div(100, 7), 14);
    EXPECT_EQ(arith::rem(100, 7), 2);
}
