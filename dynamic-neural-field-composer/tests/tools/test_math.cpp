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
