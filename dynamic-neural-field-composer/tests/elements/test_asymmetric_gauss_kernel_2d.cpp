#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 20, int y_max = 20)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static AsymmetricGaussKernel2DParameters makeAGKP(double width = 3.0, double amp = 3.0,
    double ampG = 0.0, double tsX = 0.0, double tsY = 0.0,
    bool circ = true, bool norm = true)
{
    return AsymmetricGaussKernel2DParameters{ width, amp, ampG, tsX, tsY, circ, norm };
}

TEST(AsymmetricGaussKernel2DConstruction, LabelIsCorrect)
{
    AsymmetricGaussKernel2D agk(makeCP("agk"), makeAGKP());
    EXPECT_EQ(agk.getLabel(), ElementLabel::ASYMMETRIC_GAUSS_KERNEL_2D);
}

TEST(AsymmetricGaussKernel2DConstruction, SizeIsProductOfDimensions)
{
    AsymmetricGaussKernel2D agk(makeCP("agk", 8, 6), makeAGKP());
    EXPECT_EQ(agk.getSize(), 48);
}

TEST(AsymmetricGaussKernel2DStep, OutputSizeMatchesDimensions)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto agk = std::make_shared<AsymmetricGaussKernel2D>(
        makeCP("agk", 20, 20), makeAGKP(3.0, 1.0, 0.0, 0.0, 0.0, false, false));
    agk->addInput(stimulus);
    agk->init();
    agk->step(0.0, 1.0);

    EXPECT_EQ(static_cast<int>(agk->getComponent("output").size()), 400);
}

TEST(AsymmetricGaussKernel2DStep, PositiveAmplitudeGivesPositiveOutput)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto agk = std::make_shared<AsymmetricGaussKernel2D>(
        makeCP("agk", 20, 20), makeAGKP(3.0, 1.0, 0.0, 0.0, 0.0, false, false));
    agk->addInput(stimulus);
    agk->init();
    agk->step(0.0, 1.0);

    const auto out = agk->getComponent("output");
    const double maxV = *std::ranges::max_element(out);
    EXPECT_GT(maxV, 0.0);
}

TEST(AsymmetricGaussKernel2DParameters, GetParametersRoundtrip)
{
    const auto p = makeAGKP(2.0, 1.5, -0.01, 0.5, 0.3, true, true);
    AsymmetricGaussKernel2D agk(makeCP("agk"), p);
    EXPECT_EQ(agk.getParameters(), p);
}

TEST(AsymmetricGaussKernel2DClone, CloneHasSameParameters)
{
    AsymmetricGaussKernel2D agk(makeCP("agk"), makeAGKP(2.5, 1.5, -0.005, 0.1, 0.2, true, true));
    agk.init();
    const auto cloned = std::dynamic_pointer_cast<AsymmetricGaussKernel2D>(agk.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), agk.getParameters());
}

TEST(AsymmetricGaussKernel2DToString, NonEmpty)
{
    AsymmetricGaussKernel2D agk(makeCP("agk"), makeAGKP());
    EXPECT_FALSE(agk.toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(AsymmetricGaussKernel2DEdgeCases, ZeroAmplitudeOutputAllZeros)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto agk = std::make_shared<AsymmetricGaussKernel2D>(
        makeCP("agk", 20, 20), makeAGKP(3.0, 0.0, 0.0, 0.0, 0.0, false, false));
    agk->addInput(stimulus);
    agk->init();
    agk->step(0.0, 1.0);

    for (double v : agk->getComponent("output"))
        EXPECT_NEAR(v, 0.0, 1e-10);
}

TEST(AsymmetricGaussKernel2DEdgeCases, OutputNoNaNOrInfAfterMultipleSteps)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto agk = std::make_shared<AsymmetricGaussKernel2D>(
        makeCP("agk", 20, 20), makeAGKP(3.0, 1.0, 0.0, 0.5, 0.5, false, false));
    agk->addInput(stimulus);
    agk->init();
    for (int i = 0; i < 10; ++i)
        agk->step(static_cast<double>(i), 1.0);

    for (double v : agk->getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}
