#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>

#include "elements/oscillatory_kernel_2d.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x = 20, int y = 20)
{
    return ElementCommonParameters{ name, ElementDimensions(x, y, 1.0, 1.0) };
}

static OscillatoryKernel2DParameters makeOKP(
    double amp = 1.0, double decay = 0.1, double zc = 0.3,
    double ampG = 0.0, bool circ = true, bool norm = false)
{
    return OscillatoryKernel2DParameters{ amp, decay, zc, ampG, circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(OscillatoryKernel2DConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(OscillatoryKernel2D(makeCP("ok2d"), makeOKP()));
}

TEST(OscillatoryKernel2DConstruction, LabelIsOscillatoryKernel2D)
{
    OscillatoryKernel2D ok(makeCP("ok2d"), makeOKP());
    EXPECT_EQ(ok.getLabel(), ElementLabel::OSCILLATORY_KERNEL_2D);
}

TEST(OscillatoryKernel2DConstruction, SizeIsProductOfDimensions)
{
    OscillatoryKernel2D ok(makeCP("ok2d", 8, 6), makeOKP());
    EXPECT_EQ(ok.getSize(), 48);
}

TEST(OscillatoryKernel2DConstruction, ZeroCrossingsClampedToOne)
{
    OscillatoryKernel2DParameters p = makeOKP(1.0, 0.1, 2.5);
    EXPECT_DOUBLE_EQ(p.zeroCrossings, 1.0);
}

TEST(OscillatoryKernel2DConstruction, ZeroCrossingsClampedToZero)
{
    OscillatoryKernel2DParameters p = makeOKP(1.0, 0.1, -0.5);
    EXPECT_DOUBLE_EQ(p.zeroCrossings, 0.0);
}

TEST(OscillatoryKernel2DConstruction, DecayClampedToPositive)
{
    OscillatoryKernel2DParameters p = makeOKP(1.0, -1.0);
    EXPECT_GT(p.decay, 0.0);
}

// ---------------------------------------------------------------------------
// step() — output size and basic values
// ---------------------------------------------------------------------------

TEST(OscillatoryKernel2DStep, OutputSizeMatchesDimensions)
{
    auto stim = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 10.0, 10.0, 10.0, false, false });
    stim->init();

    auto ok = std::make_shared<OscillatoryKernel2D>(makeCP("ok2d", 20, 20), makeOKP(1.0, 0.1, 0.3, 0.0, false, false));
    ok->addInput(stim);
    ok->init();
    ok->step(0.0, 1.0);

    EXPECT_EQ(static_cast<int>(ok->getComponent("output").size()), 400);
}

TEST(OscillatoryKernel2DStep, PositiveAmplitudeProducesNonZeroOutput)
{
    auto stim = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 10.0, 10.0, 10.0, false, false });
    stim->init();

    auto ok = std::make_shared<OscillatoryKernel2D>(makeCP("ok2d", 20, 20), makeOKP(1.0, 0.1, 0.3, 0.0, false, false));
    ok->addInput(stim);
    ok->init();
    ok->step(0.0, 1.0);

    const auto out = ok->getComponent("output");
    const double maxAbs = *std::ranges::max_element(out, [](double a, double b){ return std::abs(a) < std::abs(b); });
    EXPECT_GT(std::abs(maxAbs), 0.0);
}

TEST(OscillatoryKernel2DStep, ZeroAmplitudeGivesAllZeroOutput)
{
    auto stim = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 10.0, 10.0, 10.0, false, false });
    stim->init();

    auto ok = std::make_shared<OscillatoryKernel2D>(makeCP("ok2d", 20, 20), makeOKP(0.0, 0.1, 0.3, 0.0, false, false));
    ok->addInput(stim);
    ok->init();
    ok->step(0.0, 1.0);

    const auto out = ok->getComponent("output");
    for (const double v : out)
        EXPECT_NEAR(v, 0.0, 1e-10);
}

TEST(OscillatoryKernel2DStep, GlobalTermShiftsOutput)
{
    // A wide Gaussian stimulus produces a near-uniform input.
    // With a positive amplitudeGlobal the mean output should shift up.
    auto stim = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 10, 10),
        GaussStimulusParameters2D{ 50.0, 1.0, 5.0, 5.0, false, false });
    stim->init();

    auto okNoG = std::make_shared<OscillatoryKernel2D>(makeCP("ok1", 10, 10), makeOKP(1.0, 0.1, 0.3, 0.0,  false, false));
    auto okG   = std::make_shared<OscillatoryKernel2D>(makeCP("ok2", 10, 10), makeOKP(1.0, 0.1, 0.3, 0.5,  false, false));

    okNoG->addInput(stim); okG->addInput(stim);
    okNoG->init(); okG->init();
    okNoG->step(0.0, 1.0); okG->step(0.0, 1.0);

    const auto outNoG = okNoG->getComponent("output");
    const auto outG   = okG->getComponent("output");

    const double sumNoG = std::accumulate(outNoG.begin(), outNoG.end(), 0.0);
    const double sumG   = std::accumulate(outG.begin(),   outG.end(),   0.0);
    EXPECT_GT(sumG, sumNoG);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(OscillatoryKernel2DParameters, GetParametersRoundtrip)
{
    const auto p = makeOKP(2.0, 0.05, 0.4, -0.01, true, false);
    OscillatoryKernel2D ok(makeCP("ok2d"), p);
    EXPECT_EQ(ok.getParameters(), p);
}

TEST(OscillatoryKernel2DParameters, SetParametersApplied)
{
    OscillatoryKernel2D ok(makeCP("ok2d"), makeOKP());
    ok.init();

    const auto newP = makeOKP(3.0, 0.05, 0.2, -0.005, false, false);
    ok.setParameters(newP);
    EXPECT_EQ(ok.getParameters(), newP);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(OscillatoryKernel2DClone, CloneHasSameParameters)
{
    OscillatoryKernel2D ok(makeCP("ok2d"), makeOKP(2.0, 0.06, 0.35, -0.01, true, false));
    ok.init();
    const auto cloned = std::dynamic_pointer_cast<OscillatoryKernel2D>(ok.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), ok.getParameters());
}

TEST(OscillatoryKernel2DClone, CloneProducesSameOutput)
{
    auto stim = std::make_shared<GaussStimulus2D>(
        makeCP("gs", 20, 20),
        GaussStimulusParameters2D{ 3.0, 5.0, 10.0, 10.0, false, false });
    stim->init();

    auto ok = std::make_shared<OscillatoryKernel2D>(makeCP("ok2d", 20, 20), makeOKP(1.0, 0.1, 0.3, 0.0, false, false));
    ok->addInput(stim);
    ok->init();
    ok->step(0.0, 1.0);

    auto cloned = std::dynamic_pointer_cast<OscillatoryKernel2D>(ok->clone());
    cloned->addInput(stim);
    cloned->init();
    cloned->step(0.0, 1.0);

    const auto outOrig   = ok->getComponent("output");
    const auto outCloned = cloned->getComponent("output");

    ASSERT_EQ(outOrig.size(), outCloned.size());
    for (size_t i = 0; i < outOrig.size(); ++i)
        EXPECT_NEAR(outOrig[i], outCloned[i], 1e-10);
}
