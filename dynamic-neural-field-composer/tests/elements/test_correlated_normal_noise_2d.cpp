#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <cmath>
#include <vector>
#include <array>

#include "elements/correlated_normal_noise_2d.h"
#include "tools/math.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
using namespace dnf_composer::tools::math;

static std::shared_ptr<CorrelatedNormalNoise2D> makeCNN(const std::string& name,
                                                         double amplitude = 0.05,
                                                         double width = 2.0,
                                                         bool circular = true,
                                                         int x_max = 20, int y_max = 20)
{
    ElementCommonParameters cp{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
    CorrelatedNormalNoise2DParameters p{ amplitude, width, circular };
    return std::make_shared<CorrelatedNormalNoise2D>(cp, p);
}

TEST(CorrelatedNormalNoise2D, LabelIsCorrect)
{
    const auto n = makeCNN("cnn2d");
    EXPECT_EQ(n->getLabel(), ElementLabel::CORRELATED_NORMAL_NOISE_2D);
}

TEST(CorrelatedNormalNoise2D, SizeIsProductOfDimensions)
{
    const auto n = makeCNN("cnn2d", 0.05, 2.0, true, 10, 8);
    EXPECT_EQ(n->getSize(), 80);
}

TEST(CorrelatedNormalNoise2D, OutputSizeMatchesAfterStep)
{
    const auto n = makeCNN("cnn2d", 0.05, 2.0, true, 10, 10);
    n->init();
    n->step(0.0, 1.0);
    EXPECT_EQ(static_cast<int>(n->getComponent("output").size()), 100);
}

TEST(CorrelatedNormalNoise2D, OutputIsNonZeroAfterStep)
{
    const auto n = makeCNN("cnn2d", 1.0, 2.0, true, 10, 10);
    n->init();
    n->step(0.0, 1.0);
    const auto& out = n->getComponent("output");
    const double sum = std::accumulate(out.begin(), out.end(), 0.0,
                                       [](double a, double b) { return a + std::abs(b); });
    EXPECT_GT(sum, 0.0);
}

TEST(CorrelatedNormalNoise2D, ZeroAmplitudeGivesZeroOutput)
{
    const auto n = makeCNN("cnn2d", 0.0, 2.0, true, 10, 10);
    n->init();
    n->step(0.0, 1.0);
    for (const double v : n->getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(CorrelatedNormalNoise2D, GetSetParameters)
{
    const auto n = makeCNN("cnn2d");
    CorrelatedNormalNoise2DParameters p{ 0.1, 3.0, false };
    n->setParameters(p);
    const auto got = n->getParameters();
    EXPECT_NEAR(got.amplitude, 0.1, 1e-9);
    EXPECT_NEAR(got.width, 3.0, 1e-9);
    EXPECT_FALSE(got.circular);
}

TEST(CorrelatedNormalNoise2D, ScalesWithAmplitude)
{
    constexpr int steps = 200;
    constexpr double dt = 1.0;

    auto low  = makeCNN("low",  0.1, 1.0, false, 10, 10);
    auto high = makeCNN("high", 1.0, 1.0, false, 10, 10);
    low->init();  high->init();

    double varLow = 0.0, varHigh = 0.0;
    for (int s = 0; s < steps; ++s)
    {
        low->step(0.0, dt);  high->step(0.0, dt);
        for (const double v : low->getComponent("output"))  varLow  += v * v;
        for (const double v : high->getComponent("output")) varHigh += v * v;
    }
    EXPECT_GT(varHigh / varLow, 10.0);
}

TEST(CorrelatedNormalNoise2D, CircularAndNonCircularSameSizeOutput)
{
    const auto circ    = makeCNN("circ",    0.05, 2.0, true,  10, 10);
    const auto nonCirc = makeCNN("noncirc", 0.05, 2.0, false, 10, 10);
    circ->init();    nonCirc->init();
    circ->step(0.0, 1.0);    nonCirc->step(0.0, 1.0);
    EXPECT_EQ(circ->getComponent("output").size(), 100u);
    EXPECT_EQ(nonCirc->getComponent("output").size(), 100u);
}

TEST(CorrelatedNormalNoise2D, CloneHasSameParameters)
{
    const auto n = makeCNN("cnn2d", 0.1, 3.0, false);
    n->init();
    const auto cloned = std::dynamic_pointer_cast<CorrelatedNormalNoise2D>(n->clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), n->getParameters());
}

TEST(CorrelatedNormalNoise2D, ToStringIsNonEmpty)
{
    const auto n = makeCNN("cnn2d");
    EXPECT_FALSE(n->toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(CorrelatedNormalNoise2DEdgeCases, OutputNoNaNOrInfAfterMultipleSteps)
{
    const auto n = makeCNN("cnn2d", 0.05, 2.0, true, 10, 10);
    n->init();
    for (int i = 0; i < 10; ++i)
        n->step(static_cast<double>(i), 1.0);
    for (double v : n->getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}

// ---------------------------------------------------------------------------
// Kernel-support clamp (fixes a real out-of-bounds read: the old
// halfWidth = 5*width had no clamp to the field size, so a wide width on a
// small field -- e.g. width=3.0 on a 10x10 field -> old halfWidth=15 --
// made createExtendedIndex return a negative starting index that
// conv2d_separable_into's circular x-pass read as an offset before the row
// buffer). init() now clamps via computeKernelRange, per axis, matching
// every other 2D kernel element.
// ---------------------------------------------------------------------------

TEST(CorrelatedNormalNoise2D, WideWidthOnSmallFieldStaysInBounds)
{
    const int sx = 10, sy = 10;
    constexpr double width = 3.0; // old formula: halfWidth = 15 > field size -> OOB
    auto n = makeCNN("cnn_wide", 1.0, width, true, sx, sy);
    seedNormal(42);
    n->init();
    n->step(0.0, 1.0);
    const auto out = n->getComponent("output");
    ASSERT_EQ(out.size(), static_cast<size_t>(sx * sy));
    for (double v : out) EXPECT_TRUE(std::isfinite(v));

    // Deterministic cross-check (not just "is finite"): rebuild the same
    // clamped-taps convolution independently and compare element-wise.
    seedNormal(42);
    std::vector<double> noise(static_cast<size_t>(sx) * sy);
    fillNormal(noise.data(), noise.size());

    const auto range = computeKernelRange(width, /*cutOfFactor=*/5, sx, /*circular=*/true);
    std::vector<int> rangeVec(range[0] + range[1] + 1);
    std::iota(rangeVec.begin(), rangeVec.end(), -range[0]);
    const auto taps = gaussNorm(rangeVec, 0.0, width);
    const auto extX = createExtendedIndex(sx, range);
    const auto extY = createExtendedIndex(sy, range);

    Conv2dScratch<double> scratch;
    scratch.ensure(sx, sy, extX.size(), extY.size());
    std::vector<double> refOut(static_cast<size_t>(sx) * sy), tmp(static_cast<size_t>(sx) * sy);
    conv2d_separable_into(refOut, tmp, scratch, noise, taps, taps, sx, sy, extX, extY);

    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_NEAR(out[i], refOut[i], 1e-9) << "mismatch at " << i;
}

TEST(CorrelatedNormalNoise2D, KernelSupportClampedPerAxisOnNonSquareGrid)
{
    // sx != sy: computeKernelRange's clamp binds differently per axis, which
    // is exactly what dropping the old "correlationKernel_y = correlationKernel_x"
    // shortcut is for -- catches a regression back to a single shared range.
    const int sx = 10, sy = 20;
    constexpr double width = 3.0;
    const auto rangeX = computeKernelRange(width, 5, sx, true);
    const auto rangeY = computeKernelRange(width, 5, sy, true);
    ASSERT_NE(rangeX, rangeY) << "test precondition: axes must clamp differently";

    auto n = makeCNN("cnn_nonsquare", 1.0, width, true, sx, sy);
    seedNormal(7);
    n->init();
    n->step(0.0, 1.0);
    const auto out = n->getComponent("output");
    ASSERT_EQ(out.size(), static_cast<size_t>(sx * sy));

    seedNormal(7);
    std::vector<double> noise(static_cast<size_t>(sx) * sy);
    fillNormal(noise.data(), noise.size());

    std::vector<int> rvx(rangeX[0] + rangeX[1] + 1);
    std::iota(rvx.begin(), rvx.end(), -rangeX[0]);
    std::vector<int> rvy(rangeY[0] + rangeY[1] + 1);
    std::iota(rvy.begin(), rvy.end(), -rangeY[0]);
    const auto tapsX = gaussNorm(rvx, 0.0, width);
    const auto tapsY = gaussNorm(rvy, 0.0, width);
    const auto extX = createExtendedIndex(sx, rangeX);
    const auto extY = createExtendedIndex(sy, rangeY);

    Conv2dScratch<double> scratch;
    scratch.ensure(sx, sy, extX.size(), extY.size());
    std::vector<double> refOut(static_cast<size_t>(sx) * sy), tmp(static_cast<size_t>(sx) * sy);
    conv2d_separable_into(refOut, tmp, scratch, noise, tapsX, tapsY, sx, sy, extX, extY);

    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_NEAR(out[i], refOut[i], 1e-9) << "mismatch at " << i;
}

TEST(CorrelatedNormalNoise2D, SeededOutputIsReproducible)
{
    auto n1 = makeCNN("cnn_seed1", 1.0, 2.0, true, 20, 20);
    seedNormal(99);
    n1->init();
    n1->step(0.0, 1.0);
    const auto out1 = n1->getComponent("output");

    auto n2 = makeCNN("cnn_seed2", 1.0, 2.0, true, 20, 20);
    seedNormal(99);
    n2->init();
    n2->step(0.0, 1.0);
    const auto out2 = n2->getComponent("output");

    ASSERT_EQ(out1.size(), out2.size());
    for (size_t i = 0; i < out1.size(); ++i)
        EXPECT_DOUBLE_EQ(out1[i], out2[i]);
}
