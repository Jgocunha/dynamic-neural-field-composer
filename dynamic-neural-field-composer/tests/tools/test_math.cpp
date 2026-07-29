#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

#include "tools/math.h"

using namespace dnf_composer::tools::math;

// ---------------------------------------------------------------------------
// conv
// ---------------------------------------------------------------------------

TEST(Conv, BoxConvBox)
{
    // [1,1,1] * [1,1,1] = [1,2,3,2,1]
    const std::vector<double> f{ 1, 1, 1 };
    const std::vector<double> g{ 1, 1, 1 };
    const auto result = conv(f, g);
    ASSERT_EQ(result.size(), 5u);
    EXPECT_NEAR(result[0], 1.0, 1e-9);
    EXPECT_NEAR(result[1], 2.0, 1e-9);
    EXPECT_NEAR(result[2], 3.0, 1e-9);
    EXPECT_NEAR(result[3], 2.0, 1e-9);
    EXPECT_NEAR(result[4], 1.0, 1e-9);
}

TEST(Conv, OutputSizeIsNfPlusNgMinusOne)
{
    const std::vector<double> f(10, 1.0);
    const std::vector<double> g(5, 1.0);
    const auto result = conv(f, g);
    EXPECT_EQ(result.size(), 10u + 5u - 1u);
}

TEST(Conv, ConvWithDelta)
{
    // f * [0,0,1] should shift f
    const std::vector<double> f{ 1, 2, 3 };
    const std::vector<double> g{ 0, 0, 1 };
    const auto result = conv(f, g);
    ASSERT_GE(result.size(), 3u);
    // result[2..4] should match f
    EXPECT_NEAR(result[2], 1.0, 1e-9);
    EXPECT_NEAR(result[3], 2.0, 1e-9);
    EXPECT_NEAR(result[4], 3.0, 1e-9);
}

// ---------------------------------------------------------------------------
// conv_valid
// ---------------------------------------------------------------------------

TEST(ConvValid, OutputSizeIsAbsDiffPlusOne)
{
    const std::vector<double> f(10, 1.0);
    const std::vector<double> g(5, 1.0);
    const auto result = conv_valid(f, g);
    // max - min + 1 = 10 - 5 + 1 = 6
    EXPECT_EQ(result.size(), 6u);
}

TEST(ConvValid, KnownResult)
{
    // f=[1,2,3,4,5], g=[1,1,1] → valid convolution = [6,9,12] (sums of 3 consecutive)
    const std::vector<double> f{ 1, 2, 3, 4, 5 };
    const std::vector<double> g{ 1, 1, 1 };
    const auto result = conv_valid(f, g);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_NEAR(result[0], 6.0, 1e-9);
    EXPECT_NEAR(result[1], 9.0, 1e-9);
    EXPECT_NEAR(result[2], 12.0, 1e-9);
}

// ---------------------------------------------------------------------------
// conv_same
// ---------------------------------------------------------------------------

TEST(ConvSame, OutputSizeMatchesInputF)
{
    const std::vector<double> f(20, 1.0);
    const std::vector<double> g(5, 1.0);
    const auto result = conv_same(f, g);
    EXPECT_EQ(result.size(), f.size());
}

// ---------------------------------------------------------------------------
// gauss (rangeX overload)
// ---------------------------------------------------------------------------

TEST(Gauss, PeakAtPositionZero)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    auto g = gauss(rangeX, 0.0, 2.0);
    // peak should be at index 5 (where rangeX = 0)
    const int peakIdx = static_cast<int>(std::ranges::max_element(g) - g.begin());
    EXPECT_EQ(peakIdx, 5);
}

TEST(Gauss, ValuesArePositive)
{
    const std::vector<int> rangeX{ -3, -2, -1, 0, 1, 2, 3 };
    auto g = gauss(rangeX, 0.0, 1.0);
    for (double v : g)
        EXPECT_GT(v, 0.0);
}

TEST(Gauss, PeakValueIsOne)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    auto g = gauss(rangeX, 0.0, 2.0);
    const double peak = *std::ranges::max_element(g);
    EXPECT_NEAR(peak, 1.0, 1e-9);  // gauss(0|0,sigma) = exp(0) = 1
}

// ---------------------------------------------------------------------------
// gaussNorm
// ---------------------------------------------------------------------------

TEST(GaussNorm, SumsToOne)
{
    const std::vector<int> rangeX{ -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto g = gaussNorm(rangeX, 0.0, 2.0);
    const double sum = std::accumulate(g.begin(), g.end(), 0.0);
    EXPECT_NEAR(sum, 1.0, 1e-9);
}

TEST(GaussNorm, ValuesAreNonNegative)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    const auto g = gaussNorm(rangeX, 0.0, 1.5);
    for (double v : g)
        EXPECT_GE(v, 0.0);
}

// ---------------------------------------------------------------------------
// sigmoid (template)
// ---------------------------------------------------------------------------

TEST(Sigmoid, OutputInUnitInterval)
{
    const std::vector<double> x{ -100.0, -1.0, 0.0, 1.0, 100.0 };

    for (const auto s = sigmoid(x, 10.0, 0.0); double v : s)
    {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
    }
}

TEST(Sigmoid, IsMonotonicIncreasing)
{
    const std::vector<double> x{ -5.0, -1.0, 0.0, 1.0, 5.0 };
    const auto s = sigmoid(x, 10.0, 0.0);

    for (std::size_t i = 1; i < s.size(); ++i)
    {
        EXPECT_GE(s[i], s[i - 1]);
    }
}

TEST(Sigmoid, MidpointIsHalf)
{
    const std::vector<double> x{ 0.0 };
    const auto s = sigmoid(x, 10.0, 0.0);

    ASSERT_EQ(s.size(), 1u);
    EXPECT_NEAR(s[0], 0.5, 1e-12);
}

TEST(Sigmoid, SymmetricAroundShift)
{
    const std::vector<double> x1{ -1.0 };
    const std::vector<double> x2{ 1.0 };

    const auto s1 = sigmoid(x1, 10.0, 0.0);
    const auto s2 = sigmoid(x2, 10.0, 0.0);

    ASSERT_EQ(s1.size(), 1u);
    ASSERT_EQ(s2.size(), 1u);
    EXPECT_NEAR(s1[0] + s2[0], 1.0, 1e-12);
}

TEST(Sigmoid, AtShiftValueIsHalf)
{
    const std::vector<double> x{ 3.0 };
    const auto s = sigmoid(x, 10.0, 3.0);
    EXPECT_NEAR(s[0], 0.5, 1e-9);
}

TEST(Sigmoid, OutputSizeMatchesInput)
{
    const std::vector<double> x(30, 0.0);
    const auto s = sigmoid(x, 5.0, 0.0);
    EXPECT_EQ(s.size(), x.size());
}

// ---------------------------------------------------------------------------
// heaviside
// ---------------------------------------------------------------------------

TEST(Heaviside, OutputIsZeroOrOne)
{
    const std::vector<double> x{ -2.0, -1.0, 0.0, 0.5, 1.0, 2.0 };
    for (const auto h = heaviside(x, 0.0); const double v : h)
    {
        EXPECT_TRUE(v == 0.0 || v == 1.0);
    }
}

TEST(Heaviside, AboveThresholdIsOne)
{
    const std::vector<double> x{ 1.0, 2.0, 3.0 };
    for (const auto h = heaviside(x, 0.0); const double v : h)
        EXPECT_DOUBLE_EQ(v, 1.0);
}

TEST(Heaviside, AtOrBelowThresholdIsZero)
{
    const std::vector<double> x{ 0.0, -1.0, -2.0 };
    for (const auto h = heaviside(x, 0.0); const double v : h)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// compareVectors
// ---------------------------------------------------------------------------

TEST(CompareVectors, EqualVectors)
{
    const std::vector<double> a{ 1.0, 2.0, 3.0 };
    const std::vector<double> b{ 1.0, 2.0, 3.0 };
    EXPECT_TRUE(compareVectors(a, b, 1e-6));
}

TEST(CompareVectors, UnequalVectors)
{
    const std::vector<double> a{ 1.0, 2.0, 3.0 };
    const std::vector<double> b{ 1.0, 2.0, 4.0 };
    EXPECT_FALSE(compareVectors(a, b, 1e-6));
}

TEST(CompareVectors, DifferentSizesReturnFalse)
{
    const std::vector<double> a{ 1.0, 2.0 };
    const std::vector<double> b{ 1.0, 2.0, 3.0 };
    EXPECT_FALSE(compareVectors(a, b, 1e-6));
}

TEST(CompareVectors, WithinThresholdIsEqual)
{
    const std::vector<double> a{ 1.0, 2.0 };
    const std::vector<double> b{ 1.0 + 1e-7, 2.0 - 1e-7 };
    EXPECT_TRUE(compareVectors(a, b, 1e-6));
}

// ---------------------------------------------------------------------------
// calculateVectorSum
// ---------------------------------------------------------------------------

TEST(CalculateVectorSum, KnownSum)
{
    const std::vector<double> v{ 1.0, 2.0, 3.0, 4.0 };
    EXPECT_DOUBLE_EQ(calculateVectorSum(v), 10.0);
}

TEST(CalculateVectorSum, EmptyVectorIsZero)
{
    const std::vector<double> v;
    EXPECT_DOUBLE_EQ(calculateVectorSum(v), 0.0);
}

TEST(CalculateVectorSum, NegativeValues)
{
    const std::vector<double> v{ -1.0, -2.0, 3.0 };
    EXPECT_DOUBLE_EQ(calculateVectorSum(v), 0.0);
}

// ---------------------------------------------------------------------------
// calculateVectorAvg
// ---------------------------------------------------------------------------

TEST(CalculateVectorAvg, KnownAverage)
{
    const std::vector<double> v{ 1.0, 2.0, 3.0, 4.0 };
    EXPECT_DOUBLE_EQ(calculateVectorAvg(v), 2.5);
}

TEST(CalculateVectorAvg, EmptyVectorIsDefault)
{
    const std::vector<double> v;
    EXPECT_DOUBLE_EQ(calculateVectorAvg(v), 0.0);
}

TEST(CalculateVectorAvg, SingleElement)
{
    const std::vector<double> v{ 7.0 };
    EXPECT_DOUBLE_EQ(calculateVectorAvg(v), 7.0);
}

// ---------------------------------------------------------------------------
// calculateVectorNorm
// ---------------------------------------------------------------------------

TEST(CalculateVectorNorm, KnownNorm)
{
    // norm([3,4]) = sqrt(9+16) = 5
    const std::vector<double> v{ 3.0, 4.0 };
    EXPECT_NEAR(calculateVectorNorm(v), 5.0, 1e-9);
}

TEST(CalculateVectorNorm, AllZerosIsZero)
{
    const std::vector<double> v(10, 0.0);
    EXPECT_NEAR(calculateVectorNorm(v), 0.0, 1e-9);
}

TEST(CalculateVectorNorm, SingleElementIsAbsoluteValue)
{
    const std::vector<double> v{ -4.0 };
    EXPECT_NEAR(calculateVectorNorm(v), 4.0, 1e-9);
}

// ---------------------------------------------------------------------------
// computeKernelRange
// ---------------------------------------------------------------------------

TEST(ComputeKernelRange, CircularRangeIsPositive)
{
    const auto range = computeKernelRange(5.0, 3, 100, true);
    EXPECT_GE(range[0], 0);
    EXPECT_GE(range[1], 0);
}

TEST(ComputeKernelRange, NonCircularRangeIsPositive)
{
    const auto range = computeKernelRange(5.0, 3, 100, false);
    EXPECT_GE(range[0], 0);
    EXPECT_GE(range[1], 0);
}

TEST(ComputeKernelRange, LargerSigmaGivesLargerOrEqualRange)
{
    const auto rangeSmall = computeKernelRange(1.0, 3, 100, true);
    const auto rangeLarge = computeKernelRange(10.0, 3, 100, true);
    EXPECT_GE(rangeLarge[0], rangeSmall[0]);
    EXPECT_GE(rangeLarge[1], rangeSmall[1]);
}

// ---------------------------------------------------------------------------
// createExtendedIndex
// ---------------------------------------------------------------------------

TEST(CreateExtendedIndex, SizeIsCorrect)
{
    const auto range = computeKernelRange(5.0, 3, 100, true);
    const auto extIdx = createExtendedIndex(100, range);
    // size = initialVec.size + fieldSize + kernelRange[0]
    // initialVec.size = range[1] + 1 (or similar)
    // Just verify it is non-empty and plausible
    EXPECT_FALSE(extIdx.empty());
    EXPECT_GE(static_cast<int>(extIdx.size()), 100);
}

TEST(CreateExtendedIndex, AllValuesArePositive)
{
    const auto range = computeKernelRange(5.0, 3, 100, true);
    for (const auto extIdx = createExtendedIndex(100, range); int v : extIdx)
        EXPECT_GE(v, 1);
}

TEST(CreateExtendedIndex, AllValuesWithinFieldSize)
{
    const auto range = computeKernelRange(5.0, 3, 100, true);
    for (const auto extIdx = createExtendedIndex(100, range); int v : extIdx)
        EXPECT_LE(v, 100);
}

// ---------------------------------------------------------------------------
// generateNormalVector
// ---------------------------------------------------------------------------

TEST(GenerateNormalVector, CorrectSize)
{
    const auto v = generateNormalVector(100);
    EXPECT_EQ(static_cast<int>(v.size()), 100);
}

TEST(GenerateNormalVector, NotAllZero)
{
    auto v = generateNormalVector(100);
    const bool anyNonZero = std::ranges::any_of(v, [](const double x){ return x != 0.0; });
    EXPECT_TRUE(anyNonZero);
}

namespace {
    // mean / variance / finiteness checks for a standard-normal sample.
    void expectStandardNormal(const std::vector<double>& v)
    {
        ASSERT_FALSE(v.empty());
        double sum = 0.0, sumSq = 0.0;
        for (double x : v)
        {
            ASSERT_TRUE(std::isfinite(x)) << "non-finite sample";
            sum += x;
            sumSq += x * x;
        }
        const double n = static_cast<double>(v.size());
        const double mean = sum / n;
        const double var = sumSq / n - mean * mean;
        EXPECT_NEAR(mean, 0.0, 0.02) << "sample mean too far from 0";
        EXPECT_NEAR(var, 1.0, 0.05) << "sample variance too far from 1";
    }
}

TEST(GenerateNormalVector, IsApproximatelyStandardNormal)
{
    expectStandardNormal(generateNormalVector(100000));
}

TEST(FillNormal, IsApproximatelyStandardNormal)
{
    std::vector<double> v(100000, -42.0);
    fillNormal(v.data(), v.size());
    expectStandardNormal(v);
}

TEST(FillNormal, ZeroCountIsNoOp)
{
    std::vector<double> v;          // empty
    fillNormal(v.data(), 0);        // must not crash / write
    EXPECT_TRUE(v.empty());
}

// ---------------------------------------------------------------------------
// hebbLearningRule
// ---------------------------------------------------------------------------

TEST(HebbLearningRule, WeightsAreUpdated)
{
    std::vector<double> weights{ 0.0, 0.0, 0.0, 0.0 };
    const std::vector<double> input{ 1.0, 1.0 };
    const std::vector<double> output{ 0.5, 0.5 };
    constexpr double lr = 0.1;
    for (const auto updated = hebbLearningRule(weights, input, output, lr); double w : updated)
        EXPECT_GT(w, 0.0);
}

TEST(HebbLearningRule, EmptyInputThrows)
{
    std::vector<double> weights{ 0.0 };
    const std::vector<double> input;
    const std::vector<double> output{ 1.0 };
    EXPECT_THROW(hebbLearningRule(weights, input, output, 0.1), std::invalid_argument);
}

TEST(HebbLearningRule, SizeMismatchThrows)
{
    std::vector<double> weights{ 0.0, 0.0 };  // should be 2*2=4 for 2 inputs, 2 outputs
    const std::vector<double> input{ 1.0, 1.0 };
    const std::vector<double> output{ 1.0, 1.0 };
    EXPECT_THROW(hebbLearningRule(weights, input, output, 0.1), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// normalize (scalar)
// ---------------------------------------------------------------------------

TEST(NormalizeScalar, BelowMinReturnsZero)
{
    EXPECT_DOUBLE_EQ(normalize(-1.0, 0.0, 10.0), 0.0);
}

TEST(NormalizeScalar, AboveMaxReturnsOne)
{
    EXPECT_DOUBLE_EQ(normalize(11.0, 0.0, 10.0), 1.0);
}

TEST(NormalizeScalar, MidpointReturnHalf)
{
    EXPECT_DOUBLE_EQ(normalize(5.0, 0.0, 10.0), 0.5);
}

// ---------------------------------------------------------------------------
// gaussian_2d
// ---------------------------------------------------------------------------

TEST(Gaussian2d, PeakAtMean)
{
    const double peak = gaussian_2d(2.0, 3.0, 2.0, 3.0, 1.0, 1.0, 5.0);
    EXPECT_NEAR(peak, 5.0, 1e-9);
}

TEST(Gaussian2d, DecaysAwayFromMean)
{
    const double center = gaussian_2d(0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    const double offCenter = gaussian_2d(5.0, 5.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    EXPECT_GT(center, offCenter);
}

// ---------------------------------------------------------------------------
// wrap
// ---------------------------------------------------------------------------

TEST(Wrap, NegativeWraps)
{
    EXPECT_DOUBLE_EQ(wrap(-1.0, 10.0), 9.0);
}

TEST(Wrap, AboveMaxWraps)
{
    EXPECT_DOUBLE_EQ(wrap(10.0, 10.0), 0.0);
}

TEST(Wrap, InRangeUnchanged)
{
    EXPECT_DOUBLE_EQ(wrap(5.0, 10.0), 5.0);
}

// ---------------------------------------------------------------------------
// reduce2DAxis_into
// ---------------------------------------------------------------------------

// 3x2 y-major buffer: rows y=0 -> {1,2,3}, y=1 -> {4,5,6}.
static std::vector<double> sampleField3x2() { return { 1, 2, 3, 4, 5, 6 }; }

TEST(Reduce2DAxisInto, KeepXSum)
{
    std::vector<double> out;
    reduce2DAxis_into(out, sampleField3x2(), 3, 2, /*keepX=*/true, ReduceOp::SUM);
    EXPECT_EQ(out, (std::vector<double>{ 5, 7, 9 })); // column sums
}

TEST(Reduce2DAxisInto, KeepYAverage)
{
    std::vector<double> out;
    reduce2DAxis_into(out, sampleField3x2(), 3, 2, /*keepX=*/false, ReduceOp::AVERAGE);
    EXPECT_EQ(out, (std::vector<double>{ 2, 5 })); // row means: (1+2+3)/3, (4+5+6)/3
}

TEST(Reduce2DAxisInto, NonPositiveDimensionsYieldEmpty)
{
    std::vector<double> out{ 9, 9, 9 };
    reduce2DAxis_into(out, sampleField3x2(), 0, 2, true, ReduceOp::SUM);
    EXPECT_TRUE(out.empty());

    out = { 9, 9, 9 };
    reduce2DAxis_into(out, sampleField3x2(), -3, 2, true, ReduceOp::SUM);
    EXPECT_TRUE(out.empty());

    out = { 9, 9, 9 };
    reduce2DAxis_into(out, sampleField3x2(), 3, -2, false, ReduceOp::SUM);
    EXPECT_TRUE(out.empty());
}

TEST(Reduce2DAxisInto, FieldSmallerThanDimsYieldsEmpty)
{
    // Claims 3x2 = 6 entries but only 4 are present -> must not read OOB.
    std::vector<double> field{ 1, 2, 3, 4 };
    std::vector<double> out{ 9, 9, 9 };
    reduce2DAxis_into(out, field, 3, 2, true, ReduceOp::SUM);
    EXPECT_TRUE(out.empty());
}

// ---------------------------------------------------------------------------
// broadcast1DTo2D_into
// ---------------------------------------------------------------------------

TEST(Broadcast1DTo2DInto, AlongXRepeatsProfilePerRow)
{
    std::vector<double> out;
    broadcast1DTo2D_into(out, std::vector<double>{ 10, 20, 30 }, 3, 2, /*alongX=*/true);
    EXPECT_EQ(out, (std::vector<double>{ 10, 20, 30, 10, 20, 30 }));
}

TEST(Broadcast1DTo2DInto, AlongYRepeatsProfilePerColumn)
{
    std::vector<double> out;
    broadcast1DTo2D_into(out, std::vector<double>{ 7, 8 }, 3, 2, /*alongX=*/false);
    EXPECT_EQ(out, (std::vector<double>{ 7, 7, 7, 8, 8, 8 }));
}

TEST(Broadcast1DTo2DInto, ProfileShorterThanAxisClampsInsteadOfReadingOOB)
{
    // Profile has 2 entries but x-axis is 3 wide: last index must clamp to 20.
    std::vector<double> out;
    broadcast1DTo2D_into(out, std::vector<double>{ 10, 20 }, 3, 1, /*alongX=*/true);
    EXPECT_EQ(out, (std::vector<double>{ 10, 20, 20 }));
}

TEST(Broadcast1DTo2DInto, NonPositiveDimensionsYieldEmpty)
{
    std::vector<double> out{ 9, 9 };
    broadcast1DTo2D_into(out, std::vector<double>{ 1, 2 }, -3, 2, true);
    EXPECT_TRUE(out.empty());

    out = { 9, 9 };
    broadcast1DTo2D_into(out, std::vector<double>{ 1, 2 }, 3, 0, true);
    EXPECT_TRUE(out.empty());
}

// ---------------------------------------------------------------------------
// conv_valid_into / conv_same_into — in-place variants agree with the
// allocating versions
// ---------------------------------------------------------------------------

TEST(ConvValidInto, MatchesConvValid)
{
    const std::vector<double> f{ 1, 2, 3, 4, 5 };
    const std::vector<double> g{ 1, 1, 1 };
    const auto expected = conv_valid(f, g);
    std::vector<double> out(expected.size());
    conv_valid_into(out, f, g);
    EXPECT_EQ(out, expected);
}

TEST(ConvSameInto, MatchesConvSame)
{
    const std::vector<double> f{ 1, 2, 3, 4, 5 };
    const std::vector<double> g{ 1, 1, 1 };
    const auto expected = conv_same(f, g);
    std::vector<double> out(expected.size());
    conv_same_into(out, f, g);
    EXPECT_EQ(out, expected);
}

TEST(ConvSame, KnownResult)
{
    // f=[1,2,3,4,5], g=[1,1,1], pad=1 -> zero-padded 3-box sum per position.
    const std::vector<double> f{ 1, 2, 3, 4, 5 };
    const std::vector<double> g{ 1, 1, 1 };
    const auto result = conv_same(f, g);
    ASSERT_EQ(result.size(), 5u);
    EXPECT_NEAR(result[0], 3.0, 1e-9);
    EXPECT_NEAR(result[1], 6.0, 1e-9);
    EXPECT_NEAR(result[2], 9.0, 1e-9);
    EXPECT_NEAR(result[3], 12.0, 1e-9);
    EXPECT_NEAR(result[4], 9.0, 1e-9);
}

// ---------------------------------------------------------------------------
// obtainCircularVector / obtainCircularVector_into — 1-based index lookup
// ---------------------------------------------------------------------------

TEST(ObtainCircularVector, KnownPermutation)
{
    const std::vector<int> indices{ 3, 1, 2 };
    const std::vector<double> contents{ 10, 20, 30 };
    const auto result = obtainCircularVector(indices, contents);
    // indices are 1-based: contents[indices[i]-1]
    EXPECT_EQ(result, (std::vector<double>{ 30, 10, 20 }));
}

TEST(ObtainCircularVectorInto, MatchesAllocatingVersion)
{
    const std::vector<int> indices{ 3, 1, 2, 2 };
    const std::vector<double> contents{ 10, 20, 30 };
    const auto expected = obtainCircularVector(indices, contents);
    std::vector<double> out(expected.size());
    obtainCircularVector_into(out, indices, contents);
    EXPECT_EQ(out, expected);
}

// ---------------------------------------------------------------------------
// gauss(size, sigma, position) overload / nonCircularGauss
// ---------------------------------------------------------------------------

TEST(GaussSizeOverload, PeakAtOneBasedPosition)
{
    // x = i+1, so position=6 -> peak at index 5.
    const auto g = gauss(11, 2.0, 6.0);
    const int peakIdx = static_cast<int>(std::ranges::max_element(g) - g.begin());
    EXPECT_EQ(peakIdx, 5);
    EXPECT_NEAR(g[peakIdx], 1.0, 1e-9);
}

TEST(NonCircularGauss, MatchesSizeOverload)
{
    const auto a = gauss(11, 2.0, 6.0);
    const auto b = nonCircularGauss<double>(11, 2.0, 6.0);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i)
        EXPECT_NEAR(a[i], b[i], 1e-9);
}

// ---------------------------------------------------------------------------
// circularGauss — periodic wrap-around
// ---------------------------------------------------------------------------

TEST(CircularGauss, PeakNearPosition)
{
    const auto g = circularGauss<double>(20, 2.0, 5.0);
    const int peakIdx = static_cast<int>(std::ranges::max_element(g) - g.begin());
    // Position is 1-based (x = i+1); peak should land at index position-1.
    EXPECT_EQ(peakIdx, 4);
}

TEST(CircularGauss, WrapsAroundEdge)
{
    // Position at the very start of the field: the far edge should show
    // elevated activation compared to the mid-field, since periodic distance
    // wraps around.
    const auto g = circularGauss<double>(20, 2.0, 1.0);
    const int lastIdx = static_cast<int>(g.size()) - 1;
    const int midIdx = static_cast<int>(g.size()) / 2;
    EXPECT_GT(g[lastIdx], g[midIdx]);
}

// ---------------------------------------------------------------------------
// gaussDerivative / gaussDerivativeNorm
// ---------------------------------------------------------------------------

TEST(GaussDerivative, ZeroAtPosition)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    const auto d = gaussDerivative(rangeX, 0.0, 2.0, 1.0);
    EXPECT_NEAR(d[5], 0.0, 1e-9); // rangeX[5] == 0 == position
}

TEST(GaussDerivative, AntisymmetricAroundPosition)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    const auto d = gaussDerivative(rangeX, 0.0, 2.0, 1.0);
    // d[6] is at rangeX=1 (distance +1), d[4] is at rangeX=-1 (distance -1)
    EXPECT_NEAR(d[6], -d[4], 1e-9);
}

TEST(GaussDerivative, NegativeAboveMean)
{
    const std::vector<int> rangeX{ 0, 1, 2, 3 };
    const auto d = gaussDerivative(rangeX, 0.0, 2.0, 1.0);
    EXPECT_LT(d[1], 0.0); // x=1 > position=0, positive amplitude -> negative slope contribution
}

TEST(GaussDerivativeNorm, SumOfAbsIsOne)
{
    const std::vector<int> rangeX{ -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 };
    const auto d = gaussDerivativeNorm(rangeX, 0.0, 2.0, 1.0);
    const double sumAbs = std::accumulate(d.begin(), d.end(), 0.0,
        [](double acc, double v) { return acc + std::abs(v); });
    EXPECT_NEAR(sumAbs, 1.0, 1e-9);
}

TEST(GaussDerivativeNorm, ZeroAmplitudeDoesNotDivideByZero)
{
    const std::vector<int> rangeX{ -2, -1, 0, 1, 2 };
    const auto d = gaussDerivativeNorm(rangeX, 0.0, 2.0, 0.0);
    for (double v : d)
        EXPECT_TRUE(std::isfinite(v));
}

// ---------------------------------------------------------------------------
// sumGauss
// ---------------------------------------------------------------------------

TEST(SumGauss, ElementWiseSum)
{
    const std::vector<double> a{ 1.0, 2.0, 3.0 };
    const std::vector<double> b{ 10.0, 20.0, 30.0 };
    const auto result = sumGauss(a, b);
    EXPECT_EQ(result, (std::vector<double>{ 11.0, 22.0, 33.0 }));
}

// ---------------------------------------------------------------------------
// absSigmoid
// ---------------------------------------------------------------------------

TEST(AbsSigmoid, MidpointIsHalf)
{
    const std::vector<double> x{ 3.0 };
    const auto s = absSigmoid(x, 10.0, 3.0);
    EXPECT_NEAR(s[0], 0.5, 1e-9);
}

TEST(AbsSigmoid, IsMonotonicIncreasing)
{
    const std::vector<double> x{ -5.0, -1.0, 0.0, 1.0, 5.0 };
    const auto s = absSigmoid(x, 10.0, 0.0);
    for (std::size_t i = 1; i < s.size(); ++i)
        EXPECT_GE(s[i], s[i - 1]);
}

TEST(AbsSigmoid, OutputInUnitInterval)
{
    const std::vector<double> x{ -100.0, -1.0, 0.0, 1.0, 100.0 };
    for (const auto s = absSigmoid(x, 10.0, 0.0); double v : s)
    {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
    }
}

TEST(AbsSigmoid, AgreesWithSigmoidFarFromShift)
{
    const std::vector<double> x{ -10.0, 10.0 };
    const auto absS = absSigmoid(x, 100.0, 0.0);
    const auto expS = sigmoid(x, 100.0, 0.0);
    for (std::size_t i = 0; i < x.size(); ++i)
        EXPECT_NEAR(absS[i], expS[i], 0.05);
}

// ---------------------------------------------------------------------------
// gaussian_2d_periodic / circular_gaussian_2d
// ---------------------------------------------------------------------------

TEST(GaussianPeriodic2d, PeakAtMean)
{
    const double peak = gaussian_2d_periodic(5.0, 5.0, 5.0, 5.0, 1.0, 3.0, 20.0, 20.0);
    EXPECT_NEAR(peak, 3.0, 1e-9);
}

TEST(GaussianPeriodic2d, WrapsAcrossBoundary)
{
    // max_x = 20: point at x=19 is periodic-distance 1 from mu_x=0 (via wrap),
    // same as a point at x=1 would be from mu_x=0 directly.
    const double wrapped = gaussian_2d_periodic(19.0, 0.0, 0.0, 0.0, 2.0, 1.0, 20.0, 20.0);
    const double direct  = gaussian_2d_periodic(1.0, 0.0, 0.0, 0.0, 2.0, 1.0, 20.0, 20.0);
    EXPECT_NEAR(wrapped, direct, 1e-9);
}

TEST(CircularGaussian2d, PeakAtMean)
{
    const double peak = circular_gaussian_2d(2.0, 3.0, 2.0, 3.0, 1.0, 4.0);
    EXPECT_NEAR(peak, 4.0, 1e-9);
}

TEST(CircularGaussian2d, IsotropicDecay)
{
    // Equal radius in different directions must give equal values.
    const double a = circular_gaussian_2d(1.0, 0.0, 0.0, 0.0, 1.0, 1.0);
    const double b = circular_gaussian_2d(0.0, 1.0, 0.0, 0.0, 1.0, 1.0);
    EXPECT_NEAR(a, b, 1e-9);
}

// ---------------------------------------------------------------------------
// Learning rules — oja, delta (Widrow-Hoff, Krogh-Hertz)
// ---------------------------------------------------------------------------

TEST(OjaLearningRule, MatchesHebbFromZeroWeights)
{
    // With w=0, the decay term (-out*in*w) vanishes, so Oja's update equals Hebb's.
    std::vector<double> ojaWeights{ 0.0, 0.0, 0.0, 0.0 };
    std::vector<double> hebbWeights{ 0.0, 0.0, 0.0, 0.0 };
    const std::vector<double> input{ 1.0, 2.0 };
    const std::vector<double> output{ 0.5, 1.5 };
    constexpr double lr = 0.1;

    const auto ojaResult = ojaLearningRule(ojaWeights, input, output, lr);
    const auto hebbResult = hebbLearningRule(hebbWeights, input, output, lr);

    ASSERT_EQ(ojaResult.size(), hebbResult.size());
    for (std::size_t i = 0; i < ojaResult.size(); ++i)
        EXPECT_NEAR(ojaResult[i], hebbResult[i], 1e-9);
}

TEST(OjaLearningRule, DecayShrinksUpdateFromNonZeroWeights)
{
    std::vector<double> weights{ 1.0, 1.0, 1.0, 1.0 };
    const std::vector<double> input{ 1.0, 1.0 };
    const std::vector<double> output{ 1.0, 1.0 };
    constexpr double lr = 0.1;

    // Hand-computed: w[0] += lr*(in[0]*out[0] - out[0]*in[0]*w[0])
    //              = 1.0 + 0.1*(1*1 - 1*1*1) = 1.0 + 0.1*(1 - 1) = 1.0
    const auto result = ojaLearningRule(weights, input, output, lr);
    EXPECT_NEAR(result[0], 1.0, 1e-9);
}

TEST(DeltaLearningRuleWidrowHoff, ZeroErrorMeansNoChange)
{
    std::vector<std::vector<double>> weights{ { 1.0, 2.0 }, { 3.0, 4.0 } };
    const auto original = weights;
    const std::vector<double> input{ 1.0, 1.0 };
    const std::vector<double> actual{ 5.0, 5.0 };
    const std::vector<double> target{ 5.0, 5.0 };

    const auto result = deltaLearningRuleWidrowHoff(weights, input, actual, target, 0.1);
    EXPECT_EQ(result, original);
}

TEST(DeltaLearningRuleWidrowHoff, HandComputedUpdate)
{
    // w[i][j] += lr * (target[j]-actual[j]) * input[i]
    std::vector<std::vector<double>> weights{ { 0.0, 0.0 } };
    const std::vector<double> input{ 2.0 };
    const std::vector<double> actual{ 1.0, 1.0 };
    const std::vector<double> target{ 2.0, 3.0 };
    constexpr double lr = 0.5;

    const auto result = deltaLearningRuleWidrowHoff(weights, input, actual, target, lr);
    // error = [1, 2]; w[0][0] += 0.5*1*2 = 1.0; w[0][1] += 0.5*2*2 = 2.0
    EXPECT_NEAR(result[0][0], 1.0, 1e-9);
    EXPECT_NEAR(result[0][1], 2.0, 1e-9);
}

TEST(DeltaLearningRuleKroghHertz, ArgumentOrderIsTargetThenActual)
{
    // Krogh-Hertz signature is (weights, input, targetOutput, actualOutput, lr) —
    // the opposite order of the (actualOutput, targetOutput) params vs Widrow-Hoff.
    // This test pins that order down.
    std::vector<std::vector<double>> weights{ { 0.0, 0.0 } };
    const std::vector<double> input{ 2.0 };
    const std::vector<double> targetOutput{ 2.0, 3.0 };
    const std::vector<double> actualOutput{ 1.0, 1.0 };
    constexpr double lr = 0.5;

    const auto result = deltaLearningRuleKroghHertz(weights, input, targetOutput, actualOutput, lr);
    EXPECT_NEAR(result[0][0], 1.0, 1e-9);
    EXPECT_NEAR(result[0][1], 2.0, 1e-9);
}

// ---------------------------------------------------------------------------
// normalize(vector) / flattenMatrix
// ---------------------------------------------------------------------------

TEST(NormalizeVector, MinIsZero)
{
    const std::vector<double> v{ 3.0, 1.0, 5.0, 2.0 };
    const auto result = normalize(v);
    ASSERT_EQ(result.size(), v.size());
    const double minVal = *std::ranges::min_element(result);
    EXPECT_NEAR(minVal, 0.0, 1e-6);
}

TEST(NormalizeVector, PreservesRelativeOrder)
{
    const std::vector<double> v{ 3.0, 1.0, 5.0, 2.0 };
    const auto result = normalize(v);
    // sigmoid is monotone increasing, so relative ordering must be preserved.
    EXPECT_LT(result[1], result[3]); // 1.0 < 2.0
    EXPECT_LT(result[3], result[0]); // 2.0 < 3.0
    EXPECT_LT(result[0], result[2]); // 3.0 < 5.0
}

TEST(FlattenMatrix, RowMajorOrder)
{
    const std::vector<std::vector<double>> m{ { 1.0, 2.0 }, { 3.0, 4.0 } };
    const auto flat = flattenMatrix(m);
    EXPECT_EQ(flat, (std::vector<double>{ 1.0, 2.0, 3.0, 4.0 }));
}

// ---------------------------------------------------------------------------
// resample / resampleInto / resampleNearestInto / resampleCubicInto
// ---------------------------------------------------------------------------

TEST(Resample, IdentityWhenSizesMatch)
{
    const std::vector<double> in{ 1.0, 2.0, 3.0 };
    const auto out = resample(in, 3);
    EXPECT_EQ(out, in);
}

TEST(Resample, PreservesEndpoints)
{
    const std::vector<double> in{ 0.0, 10.0, 20.0, 30.0 };
    const auto up = resample(in, 10);
    EXPECT_NEAR(up.front(), in.front(), 1e-9);
    EXPECT_NEAR(up.back(), in.back(), 1e-9);

    const auto down = resample(in, 2);
    EXPECT_NEAR(down.front(), in.front(), 1e-9);
    EXPECT_NEAR(down.back(), in.back(), 1e-9);
}

TEST(Resample, LinearDataStaysLinearWhenUpsampled)
{
    const std::vector<double> in{ 0.0, 2.0, 4.0, 6.0 }; // slope 2 per unit index
    const auto out = resample(in, 7);
    ASSERT_EQ(out.size(), 7u);
    // Positions 0..6 map onto original index range [0,3] linearly -> slope (6-0)/6 = 1
    for (std::size_t i = 0; i < out.size(); ++i)
        EXPECT_NEAR(out[i], static_cast<double>(i), 1e-9);
}

TEST(Resample, SingleOutputIsMiddleElement)
{
    const std::vector<double> in{ 1.0, 2.0, 3.0, 4.0, 5.0 };
    const auto out = resample(in, 1);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_NEAR(out[0], in[in.size() / 2], 1e-9);
}

TEST(Resample, EmptyInputYieldsEmpty)
{
    const std::vector<double> in;
    EXPECT_TRUE(resample(in, 5).empty());
}

TEST(ResampleInto, MatchesResampleWhenSizesMatch)
{
    const std::vector<double> in{ 1.0, 2.0, 3.0, 4.0 };
    std::vector<double> out(6);
    resampleInto(in, out);
    const auto expected = resample(in, 6);
    for (std::size_t i = 0; i < out.size(); ++i)
        EXPECT_NEAR(out[i], expected[i], 1e-9);
}

TEST(ResampleNearestInto, OutputValuesAreSubsetOfInput)
{
    const std::vector<double> in{ 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> out(9);
    resampleNearestInto(in, out);
    for (double v : out)
        EXPECT_NE(std::ranges::find(in, v), in.end());
}

TEST(ResampleNearestInto, CopiesWhenSizesMatch)
{
    const std::vector<double> in{ 1.0, 2.0, 3.0 };
    std::vector<double> out(3);
    resampleNearestInto(in, out);
    EXPECT_EQ(out, in);
}

TEST(ResampleCubicInto, ReproducesLinearDataExactlyInInterior)
{
    // Catmull-Rom is exactly linear wherever all four control points
    // (clamp(lo-1)..clamp(lo+2)) fall strictly inside the input range;
    // near the edges the boundary clamp duplicates a control point, which
    // breaks exactness there. With N=5 input samples and M=9 outputs,
    // pos(i)=i/2 and lo=floor(pos); the unclamped window requires
    // lo in [1,2], i.e. i in [2,5].
    const std::vector<double> in{ 0.0, 2.0, 4.0, 6.0, 8.0 };
    std::vector<double> out(9);
    resampleCubicInto(in, out);
    for (std::size_t i = 2; i <= 5; ++i)
        EXPECT_NEAR(out[i], static_cast<double>(i), 1e-9);
}

TEST(ResampleCubicInto, InterpolatesThroughSamplePoints)
{
    const std::vector<double> in{ 1.0, 5.0, 2.0, 8.0 };
    std::vector<double> out(in.size()); // same size -> should just copy
    resampleCubicInto(in, out);
    for (std::size_t i = 0; i < out.size(); ++i)
        EXPECT_NEAR(out[i], in[i], 1e-9);
}

// ---------------------------------------------------------------------------
// conv2d_separable / conv2d_separable_into
// ---------------------------------------------------------------------------

TEST(Conv2dSeparable, DeltaKernelIsIdentity)
{
    // Non-circular delta kernels [0,1,0] along both axes must reproduce the input.
    const std::vector<double> field{
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    };
    const std::vector<double> kx{ 0, 1, 0 };
    const std::vector<double> ky{ 0, 1, 0 };
    const auto result = conv2d_separable(field, kx, ky, 3, 3, {}, {});
    for (std::size_t i = 0; i < field.size(); ++i)
        EXPECT_NEAR(result[i], field[i], 1e-9);
}

TEST(Conv2dSeparable, BoxBlurOnOneHotField)
{
    // 3x3 field with a single 1 in the centre; box blur [1,1,1]x[1,1,1] (non-circular,
    // zero-padded) spreads it to all 9 cells with value 1 (each cell's 3x3 same-mode
    // window touches the centre exactly once through the separable two-pass sum).
    std::vector<double> field(9, 0.0);
    field[4] = 1.0; // centre (y=1,x=1)
    const std::vector<double> kx{ 1, 1, 1 };
    const std::vector<double> ky{ 1, 1, 1 };
    const auto result = conv2d_separable(field, kx, ky, 3, 3, {}, {});

    // Every cell within the 3x3 same-convolution window of the centre receives
    // exactly one contribution from the one-hot value -> all cells equal 1.
    for (double v : result)
        EXPECT_NEAR(v, 1.0, 1e-9);
}

TEST(Conv2dSeparable, AsymmetricKernelShiftsAlongIntendedAxis)
{
    // conv_same computes out[i] = sum_j f[i+j-pad]*g[j] with pad=(ng-1)/2=1.
    // Kernel [0,0,1] (only j=2 nonzero) gives out[i] = f[i+2-1] = f[i+1]:
    // mass at input index k surfaces at output index k-1 (a shift toward
    // smaller x). Applying it only along x (identity along y, kernel [0,1,0])
    // must confine that shift to the x axis and leave y untouched.
    std::vector<double> field(9, 0.0);
    field[4] = 1.0; // (y=1, x=1)
    const std::vector<double> kx{ 0, 0, 1 };
    const std::vector<double> ky{ 0, 1, 0 };
    const auto result = conv2d_separable(field, kx, ky, 3, 3, {}, {});

    // Row y=1: original mass at x=1 must have shifted to x=0; y unchanged.
    EXPECT_NEAR(result[3], 1.0, 1e-9); // (y=1,x=0)
    EXPECT_NEAR(result[4], 0.0, 1e-9); // (y=1,x=1)
    EXPECT_NEAR(result[1], 0.0, 1e-9); // (y=0,x=1) must stay untouched
    EXPECT_NEAR(result[7], 0.0, 1e-9); // (y=2,x=1) must stay untouched
}

TEST(Conv2dSeparableInto, MatchesAllocatingVersion)
{
    const std::vector<double> field{
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    };
    const std::vector<double> kx{ 1, 1, 1 };
    const std::vector<double> ky{ 1, 1, 1 };
    const auto expected = conv2d_separable(field, kx, ky, 4, 4, {}, {});

    std::vector<double> out(16, 0.0);
    std::vector<double> tmp(16, 0.0);
    conv2d_separable_into(out, tmp, field, kx, ky, 4, 4, {}, {});

    for (std::size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(out[i], expected[i], 1e-9);
}

TEST(Conv2dSeparableInto, CircularModeMatchesAllocatingVersion)
{
    constexpr int size = 6;
    const auto range = computeKernelRange(1.0, 1, size, true);
    const auto extIdx = createExtendedIndex(size, range);

    std::vector<double> field(size * size, 0.0);
    field[0] = 1.0;
    const std::vector<double> kx{ 1, 1, 1 };
    const std::vector<double> ky{ 1, 1, 1 };

    const auto expected = conv2d_separable(field, kx, ky, size, size, extIdx, extIdx);

    std::vector<double> out(size * size, 0.0);
    std::vector<double> tmp(size * size, 0.0);
    conv2d_separable_into(out, tmp, field, kx, ky, size, size, extIdx, extIdx);

    for (std::size_t i = 0; i < expected.size(); ++i)
        EXPECT_NEAR(out[i], expected[i], 1e-9);
}

// ---------------------------------------------------------------------------
// conv2d_separable_into — optimized in-place path must match the original
// allocating conv2d_separable bit-for-bit (covers AVX2 / branch-free interior /
// symmetric-kernel folding refactors). Reference = the untouched
// conv2d_separable (uses the original conv_valid / conv_same).
// ---------------------------------------------------------------------------

namespace {
    // Build a normalized symmetric Gaussian tap vector of given half-range.
    std::vector<double> gaussianTaps(int half, double sigma)
    {
        std::vector<int> r(2 * half + 1);
        std::iota(r.begin(), r.end(), -half);
        return gaussNorm(r, 0.0, sigma);
    }

    void expectConv2dMatchesReference(const std::vector<double>& field,
        const std::vector<double>& kx, const std::vector<double>& ky,
        int sx, int sy, bool circular)
    {
        std::vector<int> extX, extY;
        if (circular)
        {
            // Symmetric kernel range: half = (size-1)/2 of the tap vector.
            const std::array<int, 2> rx{ (static_cast<int>(kx.size()) - 1) / 2,
                                         (static_cast<int>(kx.size()) - 1) / 2 };
            const std::array<int, 2> ry{ (static_cast<int>(ky.size()) - 1) / 2,
                                         (static_cast<int>(ky.size()) - 1) / 2 };
            extX = createExtendedIndex(sx, rx);
            extY = createExtendedIndex(sy, ry);
        }

        const auto reference = conv2d_separable(field, kx, ky, sx, sy, extX, extY);

        std::vector<double> out(sx * sy), tmp(sx * sy);
        conv2d_separable_into(out, tmp, field, kx, ky, sx, sy, extX, extY);

        ASSERT_EQ(out.size(), reference.size());
        for (size_t i = 0; i < out.size(); ++i)
            EXPECT_NEAR(out[i], reference[i], 1e-12) << "mismatch at " << i;
    }

    std::vector<double> ramp(int n)
    {
        std::vector<double> v(n);
        for (int i = 0; i < n; ++i) v[i] = std::sin(0.3 * i) + 0.1 * i;
        return v;
    }
}

TEST(Conv2dSeparableInto, CircularSymmetricGaussianMatchesReference)
{
    const int sx = 50, sy = 50;
    const auto kx = gaussianTaps(9, 3.0); // 19 symmetric taps, like the benchmark kernel
    expectConv2dMatchesReference(ramp(sx * sy), kx, kx, sx, sy, /*circular=*/true);
}

TEST(Conv2dSeparableInto, NonCircularSymmetricGaussianMatchesReference)
{
    const int sx = 40, sy = 30;
    const auto kx = gaussianTaps(6, 2.0);
    const auto ky = gaussianTaps(4, 1.5);
    expectConv2dMatchesReference(ramp(sx * sy), kx, ky, sx, sy, /*circular=*/false);
}

TEST(Conv2dSeparableInto, AsymmetricKernelMatchesReference)
{
    // Non-symmetric kernel (exercises the non-folded path); odd length.
    const std::vector<double> kx{ 0.1, 0.2, 0.4, 0.2, 0.05, 0.05, 0.0 };
    const std::vector<double> ky{ 0.3, 0.5, 0.2 };
    expectConv2dMatchesReference(ramp(20 * 16), kx, ky, 20, 16, /*circular=*/true);
    expectConv2dMatchesReference(ramp(20 * 16), kx, ky, 20, 16, /*circular=*/false);
}

TEST(Conv2dSeparableInto, EvenLengthKernelMatchesReference)
{
    // Even tap count -> symmetric-folding fast path must be skipped.
    const std::vector<double> kx{ 0.25, 0.25, 0.25, 0.25 };
    expectConv2dMatchesReference(ramp(15 * 12), kx, kx, 15, 12, /*circular=*/false);
}

TEST(ConvValidInto, SymmetricFoldingMatchesNaive)
{
    // Direct check of the symmetric fold in conv_valid_into.
    const std::vector<double> ext = ramp(30);
    const auto k = gaussianTaps(5, 2.0); // 11 symmetric taps
    const int n = static_cast<int>(ext.size()) - static_cast<int>(k.size()) + 1;
    std::vector<double> got(n);
    conv_valid_into(got, ext, k);
    const auto ref = conv_valid(ext, k);
    ASSERT_EQ(ref.size(), got.size());
    for (size_t i = 0; i < got.size(); ++i)
        EXPECT_NEAR(got[i], ref[i], 1e-12);
}

// ---------------------------------------------------------------------------
// embedWrapped1D / buildWrappedSeparableKernel2D — the shared wrap-embedding
// helpers behind SpectralConvolver2D's kernel spectrum (see
// tests/tools/test_fft_convolution.cpp for the direct-vs-spectral pins that
// exercise these through the actual FFT).
// ---------------------------------------------------------------------------

TEST(EmbedWrapped1D, PlacesZeroOffsetAtIndexZero)
{
    std::vector<double> out(5, 0.0);
    const std::vector<double> window{ 7.0 }; // single tap, offset 0
    embedWrapped1D(out, window, /*kR0=*/0);
    EXPECT_DOUBLE_EQ(out[0], 7.0);
    for (size_t i = 1; i < out.size(); ++i) EXPECT_DOUBLE_EQ(out[i], 0.0);
}

TEST(EmbedWrapped1D, WrapsNegativeOffsetsToTail)
{
    // window = {a,b,c} at kR0=1 -> offsets {-1,0,+1} -> out = {b,c,0,0,a} on N=5.
    std::vector<double> out(5, 0.0);
    const std::vector<double> window{ 1.0, 2.0, 3.0 }; // a=1,b=2,c=3
    embedWrapped1D(out, window, /*kR0=*/1);
    EXPECT_DOUBLE_EQ(out[0], 2.0); // b: offset 0
    EXPECT_DOUBLE_EQ(out[1], 3.0); // c: offset +1
    EXPECT_DOUBLE_EQ(out[2], 0.0);
    EXPECT_DOUBLE_EQ(out[3], 0.0);
    EXPECT_DOUBLE_EQ(out[4], 1.0); // a: offset -1 -> N-1
}

TEST(EmbedWrapped1D, AsymmetricRangeKR0NotEqualKR1)
{
    // computeKernelRange's even-N circular clamp can yield kR0=49, kR1=50 on
    // N=100 (see ComputeKernelRange tests below) -- exercise that shape here:
    // a full-support window (100 taps) with kR0=49 must place every offset at
    // a distinct index with none dropped or aliased twice.
    const int N = 100, kR0 = 49;
    std::vector<double> window(100);
    for (int j = 0; j < 100; ++j) window[j] = static_cast<double>(j + 1); // 1..100, all distinct
    std::vector<double> out(N, 0.0);
    embedWrapped1D(out, window, kR0);

    double sum = 0.0;
    for (double v : out)
    {
        EXPECT_GT(v, 0.0); // every index hit exactly once, no zeros left
        sum += v;
    }
    double expectedSum = 0.0;
    for (double v : window) expectedSum += v;
    EXPECT_DOUBLE_EQ(sum, expectedSum);
}

TEST(EmbedWrapped1D, AccumulatesWhenWindowLongerThanField)
{
    // window longer than N: multiple taps alias onto the same index and must
    // sum (+=), not overwrite.
    const int N = 4;
    std::vector<double> window(8, 1.0); // 8 taps of value 1, kR0=0 -> offsets 0..7 mod 4
    std::vector<double> out(N, 0.0);
    embedWrapped1D(out, window, /*kR0=*/0);
    for (double v : out) EXPECT_DOUBLE_EQ(v, 2.0); // each index hit by exactly 2 taps
}

TEST(BuildWrappedSeparableKernel2D, SingleTermEqualsOuterProductOfEmbeddings)
{
    const int sx = 6, sy = 4;
    const std::vector<double> taps_x{ 1.0, 2.0, 1.0 };
    const std::vector<double> taps_y{ 0.5, 1.0 };

    std::vector<double> wx(sx, 0.0), wy(sy, 0.0);
    embedWrapped1D(wx, taps_x, 1);
    embedWrapped1D(wy, taps_y, 0);

    const SeparableKernelTerm2D term{ taps_x, 1, taps_y, 0, +1.0 };
    const auto combined = buildWrappedSeparableKernel2D(sx, sy, { term });

    ASSERT_EQ(combined.size(), static_cast<size_t>(sx * sy));
    for (int y = 0; y < sy; ++y)
        for (int x = 0; x < sx; ++x)
            EXPECT_NEAR(combined[y * sx + x], wx[x] * wy[y], 1e-15)
                << "mismatch at (" << x << "," << y << ")";
}

TEST(BuildWrappedSeparableKernel2D, TwoSignedTermsEqualDifference)
{
    const int sx = 8, sy = 8;
    const std::vector<double> excX{ 3.0, 5.0, 3.0 }, excY{ 2.0, 4.0, 2.0 };
    const std::vector<double> inhX{ 1.0, 1.0, 1.0, 1.0, 1.0 }, inhY{ 1.0, 1.0, 1.0, 1.0, 1.0 };

    const auto combined = buildWrappedSeparableKernel2D(sx, sy,
        { SeparableKernelTerm2D{ excX, 1, excY, 1, +1.0 },
          SeparableKernelTerm2D{ inhX, 2, inhY, 2, -1.0 } });

    std::vector<double> wxE(sx, 0.0), wyE(sy, 0.0), wxI(sx, 0.0), wyI(sy, 0.0);
    embedWrapped1D(wxE, excX, 1); embedWrapped1D(wyE, excY, 1);
    embedWrapped1D(wxI, inhX, 2); embedWrapped1D(wyI, inhY, 2);

    ASSERT_EQ(combined.size(), static_cast<size_t>(sx * sy));
    for (int y = 0; y < sy; ++y)
        for (int x = 0; x < sx; ++x)
        {
            const double expected = wxE[x] * wyE[y] - wxI[x] * wyI[y];
            EXPECT_NEAR(combined[y * sx + x], expected, 1e-14)
                << "mismatch at (" << x << "," << y << ")";
        }
}

TEST(BuildWrappedSeparableKernel2D, NonSquareFieldRowMajorLayout)
{
    // sx != sy: catches an x/y transpose bug that a square-field test cannot.
    const int sx = 10, sy = 3;
    const std::vector<double> taps_x{ 1.0, 2.0, 3.0 }; // len 3, kR0=1
    const std::vector<double> taps_y{ 5.0 };           // len 1, kR0=0

    const auto combined = buildWrappedSeparableKernel2D(sx, sy,
        { SeparableKernelTerm2D{ taps_x, 1, taps_y, 0, +1.0 } });
    ASSERT_EQ(combined.size(), static_cast<size_t>(sx * sy));

    // taps_y is a single tap at offset 0 -> wy = {5,0,0} (length sy=3).
    // taps_x embeds {1,2,3} at kR0=1 -> offsets {-1,0,+1} -> wx over length
    // sx=10: index0=2 (offset0), index1=3 (offset+1), index9=1 (offset-1).
    for (int y = 0; y < sy; ++y)
        for (int x = 0; x < sx; ++x)
        {
            double expected = 0.0;
            if (y == 0)
            {
                if (x == 0) expected = 2.0 * 5.0;
                else if (x == 1) expected = 3.0 * 5.0;
                else if (x == 9) expected = 1.0 * 5.0;
            }
            EXPECT_NEAR(combined[y * sx + x], expected, 1e-15)
                << "mismatch at (" << x << "," << y << ")";
        }
}

TEST(BuildWrappedSeparableKernel2D, EmptyTermListYieldsZeros)
{
    const auto combined = buildWrappedSeparableKernel2D(4, 4, std::span<const SeparableKernelTerm2D>{});
    ASSERT_EQ(combined.size(), 16u);
    for (double v : combined) EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BuildWrappedSeparableKernel2D, NonPositiveDimensionsYieldEmpty)
{
    const std::vector<double> taps{ 1.0 };
    EXPECT_TRUE(buildWrappedSeparableKernel2D(0, 4, { SeparableKernelTerm2D{ taps, 0, taps, 0, 1.0 } }).empty());
    EXPECT_TRUE(buildWrappedSeparableKernel2D(4, -1, { SeparableKernelTerm2D{ taps, 0, taps, 0, 1.0 } }).empty());
}

// ---------------------------------------------------------------------------
// seedNormal — deterministic re-seed of fillNormal's thread_local generator.
// ---------------------------------------------------------------------------

TEST(SeedNormal, SameSeedSameSequence)
{
    seedNormal(12345);
    std::vector<double> a(20);
    fillNormal(a.data(), a.size());

    seedNormal(12345);
    std::vector<double> b(20);
    fillNormal(b.data(), b.size());

    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
        EXPECT_DOUBLE_EQ(a[i], b[i]);
}

TEST(SeedNormal, DifferentSeedDifferentSequence)
{
    seedNormal(1);
    std::vector<double> a(20);
    fillNormal(a.data(), a.size());

    seedNormal(2);
    std::vector<double> b(20);
    fillNormal(b.data(), b.size());

    bool anyDifferent = false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) { anyDifferent = true; break; }
    EXPECT_TRUE(anyDifferent);
}
