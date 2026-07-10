#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <cmath>

#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 10, int y_max = 10)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static NeuralField2DParameters makeNFP(double tau = 25.0, double rl = -5.0)
{
    return NeuralField2DParameters{ tau, rl, SigmoidFunction(0.0, 10.0) };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(NeuralField2DConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(NeuralField2D(makeCP("nf"), makeNFP()));
}

TEST(NeuralField2DConstruction, LabelIsNeuralField2D)
{
    NeuralField2D nf(makeCP("nf"), makeNFP());
    EXPECT_EQ(nf.getLabel(), ElementLabel::NEURAL_FIELD_2D);
}

TEST(NeuralField2DConstruction, SizeIsProductOfDimensions)
{
    NeuralField2D nf(makeCP("nf", 8, 5), makeNFP());
    EXPECT_EQ(nf.getSize(), 8 * 5);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(NeuralField2DInit, ActivationInitialisesToRestingLevel)
{
    NeuralField2D nf(makeCP("nf", 5, 5), makeNFP(25.0, -3.0));
    nf.init();
    const auto act = nf.getComponent("activation");
    EXPECT_EQ(static_cast<int>(act.size()), 25);
    for (double v : act)
        EXPECT_NEAR(v, -3.0, 1e-9);
}

TEST(NeuralField2DInit, OutputComponentHasCorrectSize)
{
    NeuralField2D nf(makeCP("nf", 6, 4), makeNFP());
    nf.init();
    const auto out = nf.getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 24);
}

// ---------------------------------------------------------------------------
// step() — integration drives activation toward resting level + input
// ---------------------------------------------------------------------------

TEST(NeuralField2DStep, ZeroInputActivationConvergesToRestingLevel)
{
    // With no input, activation should stay at resting level (init starts there).
    NeuralField2D nf(makeCP("nf", 4, 4), makeNFP(25.0, -5.0));
    nf.init();
    for (int i = 0; i < 50; ++i)
        nf.step(static_cast<double>(i), 1.0);

    const auto act = nf.getComponent("activation");
    for (double v : act)
        EXPECT_NEAR(v, -5.0, 1e-6);
}

TEST(NeuralField2DStep, OutputSizeMatchesDimensionsAfterStep)
{
    auto nf = std::make_shared<NeuralField2D>(makeCP("nf", 5, 5), makeNFP());
    nf->init();
    nf->step(0.0, 1.0);
    EXPECT_EQ(static_cast<int>(nf->getComponent("output").size()), 25);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(NeuralField2DParameters, GetParametersRoundtrip)
{
    const auto p = makeNFP(30.0, -4.0);
    NeuralField2D nf(makeCP("nf"), p);
    EXPECT_EQ(nf.getParameters(), p);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(NeuralField2DClone, CloneHasSameParameters)
{
    NeuralField2D nf(makeCP("nf"), makeNFP(20.0, -3.0));
    nf.init();
    const auto cloned = nf.clone();
    const auto c2d = std::dynamic_pointer_cast<NeuralField2D>(cloned);
    ASSERT_NE(c2d, nullptr);
    EXPECT_EQ(c2d->getParameters(), nf.getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(NeuralField2DToString, NonEmpty)
{
    NeuralField2D nf(makeCP("nf"), makeNFP());
    EXPECT_FALSE(nf.toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(NeuralField2DEdgeCases, ActivationRemainsFiniteAfterManySteps)
{
    NeuralField2D nf(makeCP("nf", 4, 4), makeNFP(25.0, -5.0));
    nf.init();
    for (int i = 0; i < 200; ++i)
        nf.step(static_cast<double>(i), 1.0);
    for (double v : nf.getComponent("activation"))
        EXPECT_TRUE(std::isfinite(v));
}

TEST(NeuralField2DStep, BumpCentroidNearStimulusPosition)
{
    // Drive a neural field with a strong localized stimulus at a known position.
    // After convergence the activation peak (and bump centroid) must be near
    // (position_x, position_y) — this validates y-major storage end-to-end.
    constexpr int   sz   = 30;
    constexpr double px  = 15.0;
    constexpr double py  = 20.0;

    auto stim = std::make_shared<GaussStimulus2D>(
        ElementCommonParameters{"stim", ElementDimensions(sz, sz, 1.0, 1.0)},
        GaussStimulus2DParameters{2.0, 20.0, px, py, false, false});
    stim->init();

    auto nf = std::make_shared<NeuralField2D>(
        ElementCommonParameters{"nf", ElementDimensions(sz, sz, 1.0, 1.0)},
        NeuralField2DParameters{10.0, -5.0, SigmoidFunction(0.0, 10.0)});
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    // Activation peak should be at the stimulus location in y-major index space:
    // index = yi * size_x + xi  where xi=(px-1), yi=(py-1)
    const auto act = nf->getComponent("activation");
    const int peakIdx = static_cast<int>(std::ranges::max_element(act) - act.begin());
    const int xi = peakIdx % sz;
    const int yi = peakIdx / sz;
    EXPECT_NEAR((xi + 1) * 1.0, px, 2.0);
    EXPECT_NEAR((yi + 1) * 1.0, py, 2.0);
}

TEST(NeuralField2DEdgeCases, HighRestingLevelRemainsFinite)
{
    NeuralField2D nf(makeCP("nf", 4, 4), makeNFP(25.0, 10.0));
    nf.init();
    for (int i = 0; i < 100; ++i)
        nf.step(static_cast<double>(i), 1.0);
    for (double v : nf.getComponent("activation"))
        EXPECT_TRUE(std::isfinite(v));
}

// ---------------------------------------------------------------------------
// getBumps() — 2D bump metrics (centroid, amplitude, area, velocity)
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField2D> makeField2D(const std::string& name, int sz,
                                                   double tau = 10.0, double rl = -5.0)
{
    return std::make_shared<NeuralField2D>(
        ElementCommonParameters{ name, ElementDimensions(sz, sz, 1.0, 1.0) },
        NeuralField2DParameters{ tau, rl, SigmoidFunction(0.0, 10.0) });
}

static std::shared_ptr<GaussStimulus2D> makeStim2D(const std::string& name,
                                                    double px, double py,
                                                    double sigma = 2.0, double amp = 20.0)
{
    return std::make_shared<GaussStimulus2D>(
        ElementCommonParameters{ name, ElementDimensions(30, 30, 1.0, 1.0) },
        GaussStimulus2DParameters{ sigma, amp, px, py, false, false });
}

TEST(NeuralField2DBumps, NoBumpsAtRestingLevel)
{
    NeuralField2D nf(makeCP("nf", 10, 10), makeNFP());
    nf.init();
    nf.step(0.0, 1.0);
    EXPECT_TRUE(nf.getBumps().empty());
}

TEST(NeuralField2DBumps, SingleBumpDetected)
{
    auto stim = makeStim2D("stim", 15.0, 20.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    EXPECT_EQ(nf->getBumps().size(), 1u);
}

TEST(NeuralField2DBumps, CentroidNearStimulusPosition)
{
    auto stim = makeStim2D("stim", 15.0, 20.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    const auto bumps = nf->getBumps();
    ASSERT_EQ(bumps.size(), 1u);
    EXPECT_NEAR(bumps.front().centroid_x, 15.0, 2.0);
    EXPECT_NEAR(bumps.front().centroid_y, 20.0, 2.0);
}

TEST(NeuralField2DBumps, AmplitudeMatchesHighestActivation)
{
    auto stim = makeStim2D("stim", 15.0, 15.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    const auto bumps = nf->getBumps();
    ASSERT_EQ(bumps.size(), 1u);
    EXPECT_NEAR(bumps.front().amplitude, nf->getHighestActivation(), 1e-9);
}

TEST(NeuralField2DBumps, AreaIsPositiveAndBoundedAndGrowsWithSigma)
{
    auto stimSmall = makeStim2D("stim-small", 15.0, 15.0, 2.0, 20.0);
    stimSmall->init();
    auto nfSmall = makeField2D("nf-small", 30);
    nfSmall->addInput(stimSmall);
    nfSmall->init();
    for (int i = 0; i < 200; ++i)
        nfSmall->step(static_cast<double>(i), 1.0);

    auto stimLarge = makeStim2D("stim-large", 15.0, 15.0, 4.0, 20.0);
    stimLarge->init();
    auto nfLarge = makeField2D("nf-large", 30);
    nfLarge->addInput(stimLarge);
    nfLarge->init();
    for (int i = 0; i < 200; ++i)
        nfLarge->step(static_cast<double>(i), 1.0);

    const auto bumpsSmall = nfSmall->getBumps();
    const auto bumpsLarge = nfLarge->getBumps();
    ASSERT_EQ(bumpsSmall.size(), 1u);
    ASSERT_EQ(bumpsLarge.size(), 1u);

    EXPECT_GT(bumpsSmall.front().area, 0.0);
    EXPECT_LE(bumpsSmall.front().area, 30.0 * 30.0);
    EXPECT_GT(bumpsLarge.front().area, bumpsSmall.front().area);
}

TEST(NeuralField2DBumps, TwinBumpsSeparatedByFloodFill)
{
    auto stim1 = makeStim2D("stim1", 8.0, 8.0);
    stim1->init();
    auto stim2 = makeStim2D("stim2", 22.0, 22.0);
    stim2->init();

    auto nf = makeField2D("nf", 30);
    nf->addInput(stim1);
    nf->addInput(stim2);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    const auto bumps = nf->getBumps();
    ASSERT_EQ(bumps.size(), 2u);

    // Both stimulus positions must be represented among the two bumps.
    const bool foundNear8 = std::ranges::any_of(bumps, [](const NeuralField2DBump& b) {
        return std::abs(b.centroid_x - 8.0) < 2.0 && std::abs(b.centroid_y - 8.0) < 2.0;
    });
    const bool foundNear22 = std::ranges::any_of(bumps, [](const NeuralField2DBump& b) {
        return std::abs(b.centroid_x - 22.0) < 2.0 && std::abs(b.centroid_y - 22.0) < 2.0;
    });
    EXPECT_TRUE(foundNear8);
    EXPECT_TRUE(foundNear22);
}

TEST(NeuralField2DBumps, VelocityNonZeroWhenStimulusMoves)
{
    auto stim = makeStim2D("stim", 15.0, 15.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);
    ASSERT_FALSE(nf->getBumps().empty());

    // Shift the stimulus a few units to the right; keep it small enough that
    // the bump tracks rather than splitting/disappearing.
    stim->setParameters(GaussStimulus2DParameters{ 2.0, 20.0, 18.0, 15.0, false, false });

    double maxVelocityX = 0.0;
    double maxVelocityY = 0.0;
    for (int i = 0; i < 200; ++i)
    {
        nf->step(static_cast<double>(i), 1.0);
        for (const auto& b : nf->getBumps())
        {
            maxVelocityX = std::max(maxVelocityX, std::abs(b.velocity_x));
            maxVelocityY = std::max(maxVelocityY, std::abs(b.velocity_y));
        }
    }

    EXPECT_GT(maxVelocityX, 1e-9);
    EXPECT_GT(maxVelocityX, maxVelocityY);
}

TEST(NeuralField2DBumps, VelocityZeroForStationaryBump)
{
    auto stim = makeStim2D("stim", 15.0, 15.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();

    for (int i = 0; i < 300; ++i)
        nf->step(static_cast<double>(i), 1.0);

    const auto bumps = nf->getBumps();
    ASSERT_EQ(bumps.size(), 1u);
    EXPECT_NEAR(bumps.front().velocity_x, 0.0, 1e-6);
    EXPECT_NEAR(bumps.front().velocity_y, 0.0, 1e-6);
}

TEST(NeuralField2DBumps, AreaGrowsWithGridSpacing)
{
    // area = cellCount * d_x * d_y. position_x/y and sigma are physical-space
    // quantities (compared against (index+1)*d_x in the stimulus and bump
    // code, and ElementDimensions(x_max, y_max, d_x, d_y) takes x_max/y_max
    // directly rather than a cell count), so to isolate the d_x*d_y effect on
    // area we must double x_max/y_max together with d_x/d_y — this keeps
    // size_x = round(x_max/d_x) (and hence cellCount) identical across both
    // fields — and scale sigma/position by the same factor to keep the bump's
    // footprint in grid-index units constant too.
    auto stim1 = std::make_shared<GaussStimulus2D>(
        ElementCommonParameters{ "stim1", ElementDimensions(30, 30, 1.0, 1.0) },
        GaussStimulus2DParameters{ 2.0, 20.0, 15.0, 15.0, false, false });
    stim1->init();
    auto nf1 = makeField2D("nf1", 30);
    nf1->addInput(stim1);
    nf1->init();
    for (int i = 0; i < 200; ++i)
        nf1->step(static_cast<double>(i), 1.0);

    auto stim2 = std::make_shared<GaussStimulus2D>(
        ElementCommonParameters{ "stim2", ElementDimensions(60, 60, 2.0, 2.0) },
        GaussStimulus2DParameters{ 4.0, 20.0, 30.0, 30.0, false, false });
    stim2->init();
    auto nf2 = std::make_shared<NeuralField2D>(
        ElementCommonParameters{ "nf2", ElementDimensions(60, 60, 2.0, 2.0) },
        NeuralField2DParameters{ 10.0, -5.0, SigmoidFunction(0.0, 10.0) });
    nf2->addInput(stim2);
    nf2->init();
    for (int i = 0; i < 200; ++i)
        nf2->step(static_cast<double>(i), 1.0);

    const auto bumps1 = nf1->getBumps();
    const auto bumps2 = nf2->getBumps();
    ASSERT_EQ(bumps1.size(), 1u);
    ASSERT_EQ(bumps2.size(), 1u);
    EXPECT_GT(bumps2.front().area, bumps1.front().area);
}

TEST(NeuralField2DMetricsToggle, DisabledSkipsStateAndBumps)
{
    auto stim = makeStim2D("stim", 15.0, 15.0);
    stim->init();
    auto nf = makeField2D("nf", 30);
    nf->addInput(stim);
    nf->init();
    nf->setComputeStateMetrics(false);
    EXPECT_FALSE(nf->getComputeStateMetrics());

    for (int i = 0; i < 200; ++i)
        nf->step(static_cast<double>(i), 1.0);

    EXPECT_TRUE(nf->getBumps().empty());
    EXPECT_DOUBLE_EQ(nf->getHighestActivation(), 0.0);
}
