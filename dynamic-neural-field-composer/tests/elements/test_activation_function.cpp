#include <gtest/gtest.h>
#include <vector>
#include <cmath>

#include "elements/activation_function.h"

using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// SigmoidFunction
// ---------------------------------------------------------------------------

TEST(SigmoidFunction, ConstructionStoresParameters)
{
    const SigmoidFunction s{ 2.0, 5.0 };
    EXPECT_DOUBLE_EQ(s.getXShift(), 2.0);
    EXPECT_DOUBLE_EQ(s.getSteepness(), 5.0);
}

TEST(SigmoidFunction, OutputAtShiftIsHalf)
{
    // sigma(x0) = 1/(1 + exp(-beta*(x0-x0))) = 1/(1+exp(0)) = 0.5
    SigmoidFunction s{ 3.0, 10.0 };
    const std::vector<double> input{ 3.0 };
    const auto out = s(input);
    EXPECT_NEAR(out[0], 0.5, 1e-9);
}

TEST(SigmoidFunction, OutputInUnitInterval)
{
    SigmoidFunction s{ 0.0, 10.0 };
    const std::vector<double> input{ -100.0, -1.0, 0.0, 1.0, 100.0 };
    for (const auto out = s(input); double v : out)
    {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
    }
}

TEST(SigmoidFunction, LargePositiveInputApproachesOne)
{
    SigmoidFunction s{ 0.0, 10.0 };
    const std::vector<double> input{ 1000.0 };
    const auto out = s(input);
    EXPECT_NEAR(out[0], 1.0, 1e-6);
}

TEST(SigmoidFunction, LargeNegativeInputApproachesZero)
{
    SigmoidFunction s{ 0.0, 10.0 };
    const std::vector<double> input{ -1000.0 };
    const auto out = s(input);
    EXPECT_NEAR(out[0], 0.0, 1e-6);
}

TEST(SigmoidFunction, OutputSizeMatchesInputSize)
{
    SigmoidFunction s{ 0.0, 1.0 };
    const std::vector<double> input(50, 0.5);
    const auto out = s(input);
    EXPECT_EQ(out.size(), input.size());
}

TEST(SigmoidFunction, EqualityOperatorSameParameters)
{
    const SigmoidFunction a{ 1.0, 5.0 };
    const SigmoidFunction b{ 1.0, 5.0 };
    EXPECT_TRUE(a == b);
}

TEST(SigmoidFunction, EqualityOperatorDifferentParameters)
{
    const SigmoidFunction a{ 1.0, 5.0 };
    const SigmoidFunction b{ 2.0, 5.0 };
    EXPECT_FALSE(a == b);
}

TEST(SigmoidFunction, CloneProducesSameOutput)
{
    SigmoidFunction s{ 1.5, 8.0 };
    const auto clone = s.clone();
    const std::vector<double> input{ -2.0, 0.0, 1.5, 5.0 };

    const auto cloneOut = (*clone)(input);
    const auto directOut = s(input);

    for (size_t i = 0; i < input.size(); ++i)
        EXPECT_DOUBLE_EQ(cloneOut[i], directOut[i]);
}

TEST(SigmoidFunction, ToStringIsNonEmpty)
{
    const SigmoidFunction s{ 0.0, 10.0 };
    EXPECT_FALSE(s.toString().empty());
}

// ---------------------------------------------------------------------------
// HeavisideFunction
// ---------------------------------------------------------------------------

TEST(HeavisideFunction, ConstructionStoresXShift)
{
    const HeavisideFunction h{ 3.0 };
    EXPECT_DOUBLE_EQ(h.getXShift(), 3.0);
}

TEST(HeavisideFunction, OutputIsOneAboveThreshold)
{
    HeavisideFunction h{ 0.0 };
    const std::vector<double> input{ 0.1, 1.0, 100.0 };
    for (const auto out = h(input); const double v : out)
        EXPECT_DOUBLE_EQ(v, 1.0);
}

TEST(HeavisideFunction, OutputIsZeroAtOrBelowThreshold)
{
    HeavisideFunction h{ 0.0 };
    const std::vector<double> input{ 0.0, -0.1, -100.0 };
    for (const auto out = h(input); const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(HeavisideFunction, CustomThreshold)
{
    HeavisideFunction h{ 5.0 };
    const std::vector<double> input{ 4.9, 5.0, 5.1 };
    const auto out = h(input);
    EXPECT_DOUBLE_EQ(out[0], 0.0);  // 4.9 <= 5.0
    EXPECT_DOUBLE_EQ(out[1], 0.0);  // 5.0 == 5.0 (not strictly greater)
    EXPECT_DOUBLE_EQ(out[2], 1.0);  // 5.1 > 5.0
}

TEST(HeavisideFunction, OutputSizeMatchesInputSize)
{
    HeavisideFunction h{ 0.0 };
    const std::vector<double> input(75, 1.0);
    const auto out = h(input);
    EXPECT_EQ(out.size(), input.size());
}

TEST(HeavisideFunction, EqualityOperatorSameParameters)
{
    const HeavisideFunction a{ 2.0 };
    const HeavisideFunction b{ 2.0 };
    EXPECT_TRUE(a == b);
}

TEST(HeavisideFunction, EqualityOperatorDifferentParameters)
{
    const HeavisideFunction a{ 1.0 };
    const HeavisideFunction b{ 2.0 };
    EXPECT_FALSE(a == b);
}

TEST(HeavisideFunction, CloneProducesSameOutput)
{
    HeavisideFunction h{ 1.0 };
    const auto clone = h.clone();
    const std::vector<double> input{ 0.5, 1.0, 1.5 };
    const auto origOut  = h(input);
    const auto cloneOut = (*clone)(input);
    for (size_t i = 0; i < input.size(); ++i)
        EXPECT_DOUBLE_EQ(origOut[i], cloneOut[i]);
}

TEST(HeavisideFunction, ToStringIsNonEmpty)
{
    HeavisideFunction h{ 0.0 };
    EXPECT_FALSE(h.toString().empty());
}

// ---------------------------------------------------------------------------
// AbsSigmoidFunction
// ---------------------------------------------------------------------------

TEST(AbsSigmoidFunction, ConstructionStoresParameters)
{
    const AbsSigmoidFunction a{ 2.0, 50.0 };
    EXPECT_DOUBLE_EQ(a.getXShift(), 2.0);
    EXPECT_DOUBLE_EQ(a.getBeta(), 50.0);
}

TEST(AbsSigmoidFunction, OutputAtShiftIsHalf)
{
    // sigma(x_shift) = 0.5*(1 + beta*0/(1+beta*0)) = 0.5
    AbsSigmoidFunction a{ 3.0, 100.0 };
    const std::vector<double> input{ 3.0 };
    const auto out = a(input);
    EXPECT_NEAR(out[0], 0.5, 1e-12);
}

TEST(AbsSigmoidFunction, OutputInUnitInterval)
{
    AbsSigmoidFunction a{ 0.0, 20.0 };
    const std::vector<double> input{ -100.0, -1.0, 0.0, 1.0, 100.0 };
    for (const auto out = a(input); double v : out)
    {
        EXPECT_GE(v, 0.0);
        EXPECT_LE(v, 1.0);
    }
}

TEST(AbsSigmoidFunction, MonotonicallyIncreasing)
{
    AbsSigmoidFunction a{ 0.0, 10.0 };
    const std::vector<double> input{ -5.0, -1.0, 0.0, 1.0, 5.0 };
    const auto out = a(input);
    for (size_t i = 1; i < out.size(); ++i)
        EXPECT_GT(out[i], out[i - 1]);
}

TEST(AbsSigmoidFunction, LargePositiveInputApproachesOne)
{
    AbsSigmoidFunction a{ 0.0, 100.0 };
    const std::vector<double> input{ 1000.0 };
    const auto out = a(input);
    EXPECT_NEAR(out[0], 1.0, 1e-3);
}

TEST(AbsSigmoidFunction, LargeNegativeInputApproachesZero)
{
    AbsSigmoidFunction a{ 0.0, 100.0 };
    const std::vector<double> input{ -1000.0 };
    const auto out = a(input);
    EXPECT_NEAR(out[0], 0.0, 1e-3);
}

TEST(AbsSigmoidFunction, HighBetaApproachesHeaviside)
{
    // At beta=1e6 the rational sigmoid should be indistinguishable from a step
    AbsSigmoidFunction a{ 0.0, 1e6 };
    const std::vector<double> input{ -0.01, 0.01 };
    const auto out = a(input);
    EXPECT_NEAR(out[0], 0.0, 1e-3);
    EXPECT_NEAR(out[1], 1.0, 1e-3);
}

TEST(AbsSigmoidFunction, ApproximatelySameasExpSigmoidAtHighBeta)
{
    // AbsSigmoid and ExpSigmoid should agree to < 0.001 for beta >= 20
    AbsSigmoidFunction absSig{ 0.0, 50.0 };
    SigmoidFunction expSig{ 0.0, 50.0 };
    const std::vector<double> input{ -2.0, -0.5, 0.0, 0.5, 2.0 };
    const auto absOut = absSig(input);
    const auto expOut = expSig(input);
    for (size_t i = 0; i < input.size(); ++i)
        EXPECT_NEAR(absOut[i], expOut[i], 0.001);
}

TEST(AbsSigmoidFunction, OutputSizeMatchesInputSize)
{
    AbsSigmoidFunction a{ 0.0, 10.0 };
    const std::vector<double> input(60, 0.5);
    const auto out = a(input);
    EXPECT_EQ(out.size(), input.size());
}

TEST(AbsSigmoidFunction, EqualityOperatorSameParameters)
{
    const AbsSigmoidFunction a{ 1.0, 50.0 };
    const AbsSigmoidFunction b{ 1.0, 50.0 };
    EXPECT_TRUE(a == b);
}

TEST(AbsSigmoidFunction, EqualityOperatorDifferentParameters)
{
    const AbsSigmoidFunction a{ 1.0, 50.0 };
    const AbsSigmoidFunction b{ 2.0, 50.0 };
    EXPECT_FALSE(a == b);
}

TEST(AbsSigmoidFunction, CloneProducesSameOutput)
{
    AbsSigmoidFunction a{ 1.5, 30.0 };
    const auto clone = a.clone();
    const std::vector<double> input{ -2.0, 0.0, 1.5, 5.0 };
    const auto cloneOut = (*clone)(input);
    const auto directOut = a(input);
    for (size_t i = 0; i < input.size(); ++i)
        EXPECT_DOUBLE_EQ(cloneOut[i], directOut[i]);
}

TEST(AbsSigmoidFunction, ToStringIsNonEmpty)
{
    const AbsSigmoidFunction a{ 0.0, 100.0 };
    EXPECT_FALSE(a.toString().empty());
}