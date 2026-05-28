#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "elements/mexican_hat_kernel_2d.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 20, int y_max = 20)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static MexicanHatKernel2DParameters makeMHKP(
    double wExc = 2.5, double aExc = 11.0,
    double wInh = 5.0, double aInh = 15.0,
    double aGlob = -0.1, bool circ = true, bool norm = true)
{
    return MexicanHatKernel2DParameters{ wExc, aExc, wInh, aInh, aGlob, circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(MexicanHatKernel2D(makeCP("mhk"), makeMHKP()));
}

TEST(MexicanHatKernel2DConstruction, LabelIsMexicanHatKernel2D)
{
    MexicanHatKernel2D mhk(makeCP("mhk"), makeMHKP());
    EXPECT_EQ(mhk.getLabel(), ElementLabel::MEXICAN_HAT_KERNEL_2D);
}

TEST(MexicanHatKernel2DConstruction, SizeIsProductOfDimensions)
{
    MexicanHatKernel2D mhk(makeCP("mhk", 8, 6), makeMHKP());
    EXPECT_EQ(mhk.getSize(), 48);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DInit, OutputSizeIsXTimesY)
{
    MexicanHatKernel2D mhk(makeCP("mhk", 10, 8), makeMHKP());
    mhk.init();
    EXPECT_EQ(static_cast<int>(mhk.getComponent("output").size()), 80);
}

TEST(MexicanHatKernel2DInit, OutputFiniteAfterInit)
{
    MexicanHatKernel2D mhk(makeCP("mhk"), makeMHKP());
    mhk.init();
    for (double v : mhk.getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}

// ---------------------------------------------------------------------------
// step() — output shape with a Gaussian input
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DStep, OutputSizeMatchesDimensions)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto mhk = std::make_shared<MexicanHatKernel2D>(makeCP("mhk", 20, 20), makeMHKP(2.5, 11.0, 5.0, 15.0, -0.1, false, false));
    mhk->addInput(stimulus);
    mhk->init();
    mhk->step(0.0, 1.0);

    EXPECT_EQ(static_cast<int>(mhk->getComponent("output").size()), 400);
}

TEST(MexicanHatKernel2DStep, CenterPositiveFlankNegative)
{
    // Non-circular, non-normalized, so the raw Mexican hat shape is visible.
    // Stimulus at a center (10, 10) of a 20x20 field.
    const auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 2.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    const auto mhk = std::make_shared<MexicanHatKernel2D>(
        makeCP("mhk", 20, 20),
        makeMHKP(2.5, 15.0, 5.0, 10.0, 0.0, false, false));
    mhk->addInput(stimulus);
    mhk->init();
    mhk->step(0.0, 1.0);

    const auto out = mhk->getComponent("output");
    const double maxV = *std::ranges::max_element(out);
    const double minV = *std::ranges::min_element(out);
    EXPECT_GT(maxV, 0.0);
    EXPECT_LT(minV, 0.0);
}

TEST(MexicanHatKernel2DStep, OutputFiniteAfterStep)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto mhk = std::make_shared<MexicanHatKernel2D>(makeCP("mhk", 20, 20), makeMHKP(2.5, 11.0, 5.0, 15.0, -0.1, false, false));
    mhk->addInput(stimulus);
    mhk->init();
    mhk->step(0.0, 1.0);

    for (double v : mhk->getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DParameters, GetParametersRoundtrip)
{
    const auto p = makeMHKP(3.0, 12.0, 6.0, 16.0, -0.05, true, true);
    MexicanHatKernel2D mhk(makeCP("mhk"), p);
    EXPECT_EQ(mhk.getParameters(), p);
}

TEST(MexicanHatKernel2DParameters, SetParametersChangesOutput)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto mhk = std::make_shared<MexicanHatKernel2D>(makeCP("mhk", 20, 20), makeMHKP());
    mhk->addInput(stimulus);
    mhk->init();
    mhk->step(0.0, 1.0);
    const auto before = mhk->getComponent("output");

    mhk->setParameters(makeMHKP(1.0, 5.0, 2.0, 8.0, 0.0, false, false));
    mhk->step(0.0, 1.0);
    const auto after = mhk->getComponent("output");
    EXPECT_NE(before, after);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DClone, CloneHasSameParameters)
{
    MexicanHatKernel2D mhk(makeCP("mhk"), makeMHKP(3.0, 12.0, 6.0, 16.0, -0.05, true, false));
    mhk.init();
    const auto cloned = std::dynamic_pointer_cast<MexicanHatKernel2D>(mhk.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), mhk.getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DToString, NonEmpty)
{
    MexicanHatKernel2D mhk(makeCP("mhk"), makeMHKP());
    EXPECT_FALSE(mhk.toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(MexicanHatKernel2DEdgeCases, ZeroBothAmplitudesOutputAllZeros)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto mhk = std::make_shared<MexicanHatKernel2D>(
        makeCP("mhk", 20, 20),
        makeMHKP(2.5, 0.0, 5.0, 0.0, 0.0, false, false));
    mhk->addInput(stimulus);
    mhk->init();
    mhk->step(0.0, 1.0);

    for (double v : mhk->getComponent("output"))
        EXPECT_NEAR(v, 0.0, 1e-10);
}

TEST(MexicanHatKernel2DEdgeCases, OutputNoNaNOrInfAfterMultipleSteps)
{
    auto stimulus = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulus2DParameters{ 3.0, 10.0, 10.0, 10.0, false, false });
    stimulus->init();

    auto mhk = std::make_shared<MexicanHatKernel2D>(makeCP("mhk", 20, 20), makeMHKP());
    mhk->addInput(stimulus);
    mhk->init();
    for (int i = 0; i < 10; ++i)
        mhk->step(static_cast<double>(i), 1.0);

    for (double v : mhk->getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}
