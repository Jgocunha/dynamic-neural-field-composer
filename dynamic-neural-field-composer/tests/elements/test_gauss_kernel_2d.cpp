#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <algorithm>

#include "elements/gauss_kernel_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 20, int y_max = 20)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static GaussKernel2DParameters makeGKP(double width = 2.0, double amp = 2.0,
    double ampG = 0.0, bool circ = true, bool norm = true)
{
    return GaussKernel2DParameters{ width, amp, ampG, circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(GaussKernel2DConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(GaussKernel2D(makeCP("gk"), makeGKP()));
}

TEST(GaussKernel2DConstruction, LabelIsGaussKernel2D)
{
    GaussKernel2D gk(makeCP("gk"), makeGKP());
    EXPECT_EQ(gk.getLabel(), ElementLabel::GAUSS_KERNEL_2D);
}

TEST(GaussKernel2DConstruction, SizeIsProductOfDimensions)
{
    GaussKernel2D gk(makeCP("gk", 8, 6), makeGKP());
    EXPECT_EQ(gk.getSize(), 48);
}

// ---------------------------------------------------------------------------
// step() — output size and values
// ---------------------------------------------------------------------------

TEST(GaussKernel2DStep, OutputSizeMatchesDimensions)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk", 20, 20), makeGKP(2.0, 1.0, 0.0, false, false));
    gk->addInput(stimulus);
    gk->init();
    gk->step(0.0, 1.0);

    EXPECT_EQ(static_cast<int>(gk->getComponent("output").size()), 400);
}

TEST(GaussKernel2DStep, PositiveAmplitudeGivesPositiveOutput)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto gk = std::make_shared<GaussKernel2D>(makeCP("gk", 20, 20), makeGKP(2.0, 1.0, 0.0, false, false));
    gk->addInput(stimulus);
    gk->init();
    gk->step(0.0, 1.0);

    const auto out = gk->getComponent("output");
    const double maxV = *std::ranges::max_element(out);
    EXPECT_GT(maxV, 0.0);
}

TEST(GaussKernel2DStep, GlobalTermAddsConstantOffset)
{
    // With a uniform input (boost), globalTerm shifts all outputs by amp * fullSum
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 10, 10),
        GaussStimulusParameters2D{ 20.0, 1.0, 5.0, 5.0, false, false });  // very wide → near-uniform
    stimulus->init();

    const double ampG = 0.5;
    auto gkNoG = std::make_shared<GaussKernel2D>(makeCP("gk1", 10, 10), makeGKP(2.0, 1.0, 0.0,   false, false));
    auto gkG   = std::make_shared<GaussKernel2D>(makeCP("gk2", 10, 10), makeGKP(2.0, 1.0, ampG,  false, false));

    gkNoG->addInput(stimulus);
    gkG->addInput(stimulus);
    gkNoG->init(); gkG->init();
    gkNoG->step(0.0, 1.0); gkG->step(0.0, 1.0);

    const auto outNoG = gkNoG->getComponent("output");
    const auto outG   = gkG->getComponent("output");

    // Every element in gkG should be larger than in gkNoG (positive global term)
    bool allLarger = true;
    for (size_t i = 0; i < outG.size(); ++i)
        if (outG[i] <= outNoG[i]) { allLarger = false; break; }
    EXPECT_TRUE(allLarger);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(GaussKernel2DParameters, GetParametersRoundtrip)
{
    const auto p = makeGKP(3.0, 2.0, -0.01, true, true);
    GaussKernel2D gk(makeCP("gk"), p);
    EXPECT_EQ(gk.getParameters(), p);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(GaussKernel2DClone, CloneHasSameParameters)
{
    GaussKernel2D gk(makeCP("gk"), makeGKP(2.5, 1.5, -0.005, true, true));
    gk.init();
    const auto cloned = std::dynamic_pointer_cast<GaussKernel2D>(gk.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), gk.getParameters());
}
