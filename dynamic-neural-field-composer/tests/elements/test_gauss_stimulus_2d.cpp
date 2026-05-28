#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "elements/gauss_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int x_max = 50, int y_max = 50)
{
    return ElementCommonParameters{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
}

static GaussStimulus2DParameters makeGSP2D(double width = 5.0, double amp = 15.0,
    double px = 25.0, double py = 25.0, bool circ = true, bool norm = false)
{
    return GaussStimulus2DParameters{ width, amp, px, py, circ, norm };
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
    // Convention: y-major storage — index = yi * size_x + xi
    // position_x=20 -> xi=19,  position_y=30 -> yi=29
    // index = 29 * 50 + 19 = 1469
    GaussStimulus2D gs(makeCP("gs", 50, 50),
        makeGSP2D(3.0, 15.0, 20.0, 30.0, false, false));
    gs.init();
    const auto out = gs.getComponent("output");
    const int peakIdx = static_cast<int>(std::ranges::max_element(out) - out.begin());
    EXPECT_NEAR(peakIdx, 29 * 50 + 19, 10);
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

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(GaussStimulus2DEdgeCases, ZeroAmplitudeOutputAllZeros)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D(5.0, 0.0));
    gs.init();
    for (double v : gs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(GaussStimulus2DEdgeCases, PositionAtXMaxThrows)
{
    EXPECT_THROW(GaussStimulus2D(makeCP("gs", 50, 50), makeGSP2D(3.0, 10.0, 50.0, 25.0)), Exception);
}

TEST(GaussStimulus2DEdgeCases, PositionAtYMaxThrows)
{
    EXPECT_THROW(GaussStimulus2D(makeCP("gs", 50, 50), makeGSP2D(3.0, 10.0, 25.0, 50.0)), Exception);
}

TEST(GaussStimulus2DEdgeCases, PositionJustBelowMaxDoesNotThrow)
{
    EXPECT_NO_THROW(GaussStimulus2D(makeCP("gs", 50, 50), makeGSP2D(3.0, 10.0, 49.0, 49.0)));
}

TEST(GaussStimulus2DEdgeCases, OutputNoNaNOrInf)
{
    GaussStimulus2D gs(makeCP("gs"), makeGSP2D());
    gs.init();
    gs.step(0.0, 1.0);
    for (double v : gs.getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}
