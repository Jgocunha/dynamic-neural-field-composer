#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <cmath>

#include "elements/correlated_normal_noise_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

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
