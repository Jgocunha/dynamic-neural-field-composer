#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <memory>

#include "elements/gauss_stimulus.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ElementCommonParameters makeCP(const std::string& name, const int size = 100)
{
    return ElementCommonParameters{ name, size };
}

static GaussStimulusParameters makeGSP(const double width = 5.0, const double amp = 15.0,
                                        const double pos = 50.0, const bool circ = true, const bool norm = false)
{
    return GaussStimulusParameters{ width, amp, pos, circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(GaussStimulusConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(GaussStimulus(makeCP("s"), makeGSP()));
}

TEST(GaussStimulusConstruction, NegativePositionThrows)
{
    EXPECT_THROW(GaussStimulus(makeCP("s"),
        makeGSP(5.0, 15.0, -1.0)), Exception);
}

TEST(GaussStimulusConstruction, PositionAtXMaxThrows)
{
    // x_max = 100, so position = 100 (>= x_max) must throw
    EXPECT_THROW(GaussStimulus(makeCP("s", 100),
        makeGSP(5.0, 15.0, 100.0)), Exception);
}

TEST(GaussStimulusConstruction, PositionJustBelowXMaxIsValid)
{
    EXPECT_NO_THROW(GaussStimulus(makeCP("s", 100),
        makeGSP(5.0, 15.0, 99.0)));
}

TEST(GaussStimulusConstruction, LabelIsGaussStimulus)
{
    GaussStimulus gs(makeCP("s"), makeGSP());
    EXPECT_EQ(gs.getLabel(), ElementLabel::GAUSS_STIMULUS);
}

// ---------------------------------------------------------------------------
// init() / output component
// ---------------------------------------------------------------------------

TEST(GaussStimulusInit, OutputComponentHasCorrectSize)
{
    GaussStimulus gs(makeCP("s", 80),
        makeGSP(5.0, 15.0, 40.0));
    gs.init();
    const auto out = gs.getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 80);
}

TEST(GaussStimulusInit, OutputValuesAreNonNegative)
{
    GaussStimulus gs(makeCP("s"), makeGSP());
    gs.init();
    const auto out = gs.getComponent("output");
    for (double v : out)
        EXPECT_GE(v, 0.0);
}

TEST(GaussStimulusInit, NonNormalizedPeakApproachesAmplitude)
{
    // Non-normalized, amplitude=20, position=50, size=100
    GaussStimulus gs(makeCP("s", 100),
        makeGSP(3.0, 20.0, 50.0,
        false, false));
    gs.init();
    auto out = gs.getComponent("output");
    const double peak = *std::ranges::max_element(out);
    EXPECT_NEAR(peak, 20.0, 1.0);  // peak should be close to amplitude
}

TEST(GaussStimulusInit, NormalizedSumEqualsAmplitude)
{
    // When normalized=true, the sum of output = amplitude (Gaussian integrates to 1, scaled by amp)
    GaussStimulus gs(makeCP("s", 100),
        makeGSP(5.0, 10.0, 50.0,
        true, true));
    gs.init();
    auto out = gs.getComponent("output");
    const double sum = std::accumulate(out.begin(), out.end(), 0.0);
    EXPECT_NEAR(sum, 10.0, 0.1);
}

TEST(GaussStimulusInit, PeakIsNearSpecifiedPosition)
{
    // position=60, size=100 → peak index near 59 (0-indexed)
    GaussStimulus gs(makeCP("s", 100), makeGSP(5.0, 15.0, 60.0,
        false, false));
    gs.init();
    auto out = gs.getComponent("output");
    const int peakIdx = static_cast<int>(std::ranges::max_element(out) - out.begin());
    // Allow ± a few indices due to integer sampling
    EXPECT_NEAR(peakIdx, 59, 5);
}

// ---------------------------------------------------------------------------
// step() — GaussStimulus step does nothing, output is constant after init
// ---------------------------------------------------------------------------

TEST(GaussStimulusStep, OutputUnchangedAfterStep)
{
    GaussStimulus gs(makeCP("s"), makeGSP());
    gs.init();
    const auto before = gs.getComponent("output");
    gs.step(1.0, 1.0);
    const auto after = gs.getComponent("output");
    EXPECT_EQ(before, after);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(GaussStimulusParameters, GetParametersRoundtrip)
{
    const auto gsp = makeGSP(4.0, 12.0, 30.0, false, false);
    const GaussStimulus gs(makeCP("s"), gsp);
    EXPECT_EQ(gs.getParameters(), gsp);
}

TEST(GaussStimulusParameters, SetParametersUpdatesOutput)
{
    GaussStimulus gs(makeCP("s"), makeGSP(5.0, 15.0, 50.0));
    gs.init();
    const auto before = gs.getComponent("output");

    const GaussStimulusParameters newP{ 5.0, 15.0, 20.0, true, false };
    gs.setParameters(newP);   // calls init() internally
    const auto after = gs.getComponent("output");

    EXPECT_NE(before, after);  // different position → different output
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(GaussStimulusClone, CloneHasSameParameters)
{
    const auto gsp = makeGSP(3.0, 8.0, 40.0);
    GaussStimulus gs(makeCP("s"), gsp);
    gs.init();
    const auto cloned = gs.clone();
    const auto clonedGS = std::dynamic_pointer_cast<GaussStimulus>(cloned);
    ASSERT_NE(clonedGS, nullptr);
    EXPECT_EQ(clonedGS->getParameters(), gs.getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(GaussStimulusToString, NonEmpty)
{
    const GaussStimulus gs(makeCP("s"), makeGSP());
    EXPECT_FALSE(gs.toString().empty());
}

// ---------------------------------------------------------------------------
// Element base: name, size, label
// ---------------------------------------------------------------------------

TEST(GaussStimulusElement, UniqueNameMatchesConstruction)
{
    const GaussStimulus gs(makeCP("my-stim"), makeGSP());
    EXPECT_EQ(gs.getUniqueName(), "my-stim");
}

TEST(GaussStimulusElement, SizeMatchesDimension)
{
    const GaussStimulus gs(makeCP("s", 60),
        makeGSP(5.0, 15.0, 30.0));
    EXPECT_EQ(gs.getSize(), 60);
}
