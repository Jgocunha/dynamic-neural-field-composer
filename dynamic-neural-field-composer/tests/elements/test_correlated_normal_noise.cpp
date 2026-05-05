#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <cmath>

#include "elements/correlated_normal_noise.h"
#include "elements/normal_noise.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static std::shared_ptr<CorrelatedNormalNoise> makeCNN(const std::string& name,
                                                       double amplitude = 0.05,
                                                       double width = 2.0,
                                                       bool circular = true,
                                                       int size = 100)
{
    ElementCommonParameters cp{ name, size };
    CorrelatedNormalNoiseParameters p{ amplitude, width, circular };
    return std::make_shared<CorrelatedNormalNoise>(cp, p);
}

TEST(CorrelatedNormalNoise, LabelIsCorrect)
{
    const auto n = makeCNN("cnn");
    EXPECT_EQ(n->getLabel(), ElementLabel::CORRELATED_NORMAL_NOISE);
}

TEST(CorrelatedNormalNoise, OutputSizeMatchesFieldSize)
{
    const auto n = makeCNN("cnn", 0.05, 2.0, true, 80);
    n->init();
    n->step(0.0, 1.0);
    EXPECT_EQ(n->getComponent("output").size(), 80u);
}

TEST(CorrelatedNormalNoise, OutputIsNonZeroAfterStep)
{
    const auto n = makeCNN("cnn", 1.0, 2.0);
    n->init();
    n->step(0.0, 1.0);
    const auto& out = n->getComponent("output");
    const double sum = std::accumulate(out.begin(), out.end(), 0.0,
                                       [](double a, double b) { return a + std::abs(b); });
    EXPECT_GT(sum, 0.0);
}

TEST(CorrelatedNormalNoise, ScalesWithAmplitude)
{
    // Output standard deviation should scale linearly with amplitude.
    // Run many steps and compare variance between two amplitude settings.
    constexpr int steps = 500;
    constexpr int sz = 100;
    constexpr double dt = 1.0;

    auto low  = makeCNN("low",  0.1, 1.0, false, sz);
    auto high = makeCNN("high", 1.0, 1.0, false, sz);
    low->init();  high->init();

    double varLow = 0.0, varHigh = 0.0;
    for (int s = 0; s < steps; ++s)
    {
        low->step(0.0, dt);  high->step(0.0, dt);
        for (const double v : low->getComponent("output"))  varLow  += v * v;
        for (const double v : high->getComponent("output")) varHigh += v * v;
    }
    // High amplitude should have roughly 100x more variance (amplitude ratio 10x → variance 100x)
    EXPECT_GT(varHigh / varLow, 50.0);
}

TEST(CorrelatedNormalNoise, SpatialCorrelationPresent)
{
    // For large width, adjacent samples should be positively correlated.
    // For width=0.01 (essentially uncorrelated) the lag-1 autocorrelation is near 0.
    constexpr int steps = 2000;
    constexpr int sz = 100;
    constexpr double dt = 1.0;

    auto corr = makeCNN("corr", 1.0, 10.0, false, sz);
    corr->init();

    double ac_corr = 0.0;
    for (int s = 0; s < steps; ++s)
    {
        corr->step(0.0, dt);
        const auto& out = corr->getComponent("output");
        for (int i = 0; i < sz - 1; ++i)
            ac_corr += out[i] * out[i + 1];
    }
    // With width=10, adjacent elements should be strongly positively correlated
    EXPECT_GT(ac_corr / steps, 0.0);
}

TEST(CorrelatedNormalNoise, GetSetParameters)
{
    const auto n = makeCNN("cnn");
    CorrelatedNormalNoiseParameters p{ 0.1, 3.0, false };
    n->setParameters(p);
    const auto got = n->getParameters();
    EXPECT_NEAR(got.amplitude, 0.1, 1e-9);
    EXPECT_NEAR(got.width, 3.0, 1e-9);
    EXPECT_FALSE(got.circular);
}

TEST(CorrelatedNormalNoise, ToStringIsNonEmpty)
{
    const auto n = makeCNN("cnn");
    EXPECT_FALSE(n->toString().empty());
}

TEST(CorrelatedNormalNoise, HigherAutocorrelationThanWhiteNoise)
{
    // CorrelatedNormalNoise with large width should have much higher lag-1
    // spatial autocorrelation than plain NormalNoise.
    constexpr int steps = 1000;
    constexpr int sz    = 100;
    constexpr double dt = 1.0;

    auto cnn = makeCNN("cnn", 1.0, 10.0, false, sz);
    cnn->init();

    ElementCommonParameters nnCp{ "nn", sz };
    NormalNoiseParameters   nnP{ 1.0 };
    auto nn = std::make_shared<NormalNoise>(nnCp, nnP);
    nn->init();

    double acCorr = 0.0, acWhite = 0.0;
    for (int s = 0; s < steps; ++s)
    {
        cnn->step(0.0, dt);
        nn->step(0.0, dt);
        const auto& outC = cnn->getComponent("output");
        const auto& outW = nn->getComponent("output");
        for (int i = 0; i < sz - 1; ++i)
        {
            acCorr += outC[i] * outC[i + 1];
            acWhite += outW[i] * outW[i + 1];
        }
    }
    // Spatially correlated noise must have substantially higher autocorrelation
    EXPECT_GT(acCorr / steps, acWhite / steps + 1.0);
}

TEST(CorrelatedNormalNoise, CircularAndNonCircularSameSizeOutput)
{
    const auto circ    = makeCNN("circ",    0.05, 2.0, true,  80);
    const auto nonCirc = makeCNN("noncirc", 0.05, 2.0, false, 80);
    circ->init();
    nonCirc->init();
    circ->step(0.0, 1.0);
    nonCirc->step(0.0, 1.0);
    EXPECT_EQ(circ->getComponent("output").size(), 80u);
    EXPECT_EQ(nonCirc->getComponent("output").size(), 80u);
}
