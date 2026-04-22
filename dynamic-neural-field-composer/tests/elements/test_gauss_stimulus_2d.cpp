#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>

#include "elements/gauss_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 50, int y_max = 50)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static GaussStimulusParameters2D makeGSP2D(double width = 5.0, double amp = 15.0,
    double px = 25.0, double py = 25.0, bool circ = true, bool norm = false)
{
    return GaussStimulusParameters2D{ width, amp, px, py, circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DConstruction, ValidDoesNotThrow)
{
    EXPECT_NO_THROW(GaussStimulus2D(makeCP("gs"), makeGSP2D()));
}

TEST(GaussStimulus2DConstruction, LabelIsGaussStimulus2D)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D());
    EXPECT_EQ(gs.getLabel(), ElementLabel::GAUSS_STIMULUS_2D);
}

// ---------------------------------------------------------------------------
// init() — output shape and content
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DInit, OutputSizeIsXTimesY)
{
    GaussStimulus2D gs(makeCP("gs", 10, 8), makeGSP2D(2.0, 5.0, 5.0, 4.0));
    gs.init();
    EXPECT_EQ(static_cast<int>(gs.getComponent("output").size()), 80);
}

TEST(GaussStimulus2DInit, OutputValuesNonNegative)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D());
    gs.init();
    const auto out = gs.getComponent("output");
    for (double v : out)
        EXPECT_GE(v, 0.0);
}

TEST(GaussStimulus2DInit, PeakNearAmplitude)
{
    GaussStimulus2D gs(makeCP("gs", 50, 50), makeGSP2D(3.0, 20.0, 25.0, 25.0, false, false));
    gs.init();
    const auto out = gs.getComponent("output");
    const double peak = *std::ranges::max_element(out);
    EXPECT_NEAR(peak, 20.0, 2.0);
}

TEST(GaussStimulus2DInit, PeakIndexNearSpecifiedPosition)
{
    // position (20, 30) on a 50x50 grid with d_x=d_y=1.0
    // index = x * size_y + y = 20 * 50 + 30 = 1030
    GaussStimulus2D gs(makeCP("gs", 50, 50),
        makeGSP2D(3.0, 15.0, 20.0, 30.0, false, false));
    gs.init();
    const auto out = gs.getComponent("output");
    const int peakIdx = static_cast<int>(std::ranges::max_element(out) - out.begin());
    EXPECT_NEAR(peakIdx, 20 * 50 + 30, 10);
}

TEST(GaussStimulus2DInit, NormalizedSumApproachesAmplitude)
{
    GaussStimulus2D gs(makeCP("gs", 50, 50),
        makeGSP2D(5.0, 10.0, 25.0, 25.0, true, true));
    gs.init();
    const auto out = gs.getComponent("output");
    const double sum = std::accumulate(out.begin(), out.end(), 0.0);
    EXPECT_NEAR(sum, 10.0, 0.5);
}

// ---------------------------------------------------------------------------
// step() — GaussStimulus2D is static after init
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DStep, OutputUnchangedAfterStep)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D());
    gs.init();
    const auto before = gs.getComponent("output");
    gs.step(1.0, 1.0);
    const auto after = gs.getComponent("output");
    EXPECT_EQ(before, after);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DParameters, GetParametersRoundtrip)
{
    const auto p = makeGSP2D(4.0, 12.0, 15.0, 20.0, false, false);
    GaussStimulus2D gs(makeCP("gs"), p);
    EXPECT_EQ(gs.getParameters(), p);
}

TEST(GaussStimulus2DParameters, SetParametersChangesOutput)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D(5.0, 15.0, 25.0, 25.0));
    gs.init();
    const auto before = gs.getComponent("output");

    gs.setParameters(makeGSP2D(5.0, 15.0, 10.0, 10.0));
    const auto after = gs.getComponent("output");
    EXPECT_NE(before, after);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DClone, CloneHasSameParameters)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D(3.0, 8.0, 12.0, 18.0));
    gs.init();
    const auto cloned = std::dynamic_pointer_cast<GaussStimulus2D>(gs.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), gs.getParameters());
}
