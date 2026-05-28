#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <cmath>

#include "elements/timed_gauss_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int xmax = 50, int ymax = 50)
{
    return ElementCommonParameters{ name, ElementDimensions{ xmax, ymax, 1.0, 1.0 } };
}

static TimedGaussStimulus2DParameters makeTGSP2(double width = 5.0, double amp = 15.0,
    double px = 25.0, double py = 25.0,
    std::vector<std::pair<double,double>> onTimes = {{0.0, 10.0}},
    bool circ = true, bool norm = false)
{
    return TimedGaussStimulus2DParameters{ width, amp, px, py, std::move(onTimes), circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(TimedGaussStimulus2D(makeCP("s"), makeTGSP2()));
}

TEST(TimedGaussStimulus2DConstruction, NegativePositionXThrows)
{
    EXPECT_THROW(TimedGaussStimulus2D(makeCP("s"),
        makeTGSP2(5.0, 15.0, -1.0, 25.0)), Exception);
}

TEST(TimedGaussStimulus2DConstruction, NegativePositionYThrows)
{
    EXPECT_THROW(TimedGaussStimulus2D(makeCP("s"),
        makeTGSP2(5.0, 15.0, 25.0, -1.0)), Exception);
}

TEST(TimedGaussStimulus2DConstruction, PositionXAtXMaxThrows)
{
    EXPECT_THROW(TimedGaussStimulus2D(makeCP("s", 50, 50),
        makeTGSP2(5.0, 15.0, 50.0, 25.0)), Exception);
}

TEST(TimedGaussStimulus2DConstruction, LabelIsTimedGaussStimulus2D)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2());
    EXPECT_EQ(tgs.getLabel(), ElementLabel::TIMED_GAUSS_STIMULUS_2D);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DInit, OutputInitiallyZero)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2());
    tgs.init();
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulus2DInit, OutputComponentHasCorrectSize)
{
    TimedGaussStimulus2D tgs(makeCP("s", 40, 30), makeTGSP2(5.0, 15.0, 20.0, 15.0));
    tgs.init();
    EXPECT_EQ(static_cast<int>(tgs.getComponent("output").size()), 40 * 30);
}

// ---------------------------------------------------------------------------
// step() — time gating
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DStep, OutputIsNonZeroDuringOnInterval)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(3.0, 1.0);
    const auto out = tgs.getComponent("output");
    EXPECT_GT(*std::ranges::max_element(out), 0.0);
}

TEST(TimedGaussStimulus2DStep, OutputIsZeroOutsideOnInterval)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(0.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulus2DStep, OutputIsZeroAfterOnInterval)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(6.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulus2DStep, OutputActiveAtIntervalBoundaries)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{2.0, 4.0}}));
    tgs.init();

    tgs.step(2.0, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }

    tgs.step(4.0, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }
}

TEST(TimedGaussStimulus2DStep, MultipleIntervalsActivateCorrectly)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{1.0, 2.0}, {5.0, 6.0}}));
    tgs.init();

    tgs.step(1.5, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }

    tgs.step(3.0, 1.0);  // between intervals
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);

    tgs.step(5.5, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }
}

TEST(TimedGaussStimulus2DStep, EmptyOnTimesAlwaysOff)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {}));
    tgs.init();
    tgs.step(100.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// Peak location matches specified position (y-major convention)
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DStep, PeakIndexNearSpecifiedPosition)
{
    // Convention: y-major storage — index = yi * size_x + xi
    // position_x=15 -> xi=14,  position_y=20 -> yi=19
    // index = 19 * 50 + 14 = 964
    TimedGaussStimulus2D tgs(makeCP("s", 50, 50),
        makeTGSP2(3.0, 15.0, 15.0, 20.0, {{0.0, 100.0}}, false, false));
    tgs.init();
    tgs.step(1.0, 1.0);
    const auto out = tgs.getComponent("output");
    const int peakIdx = static_cast<int>(std::ranges::max_element(out) - out.begin());
    EXPECT_NEAR(peakIdx, 19 * 50 + 14, 10);
}

// ---------------------------------------------------------------------------
// Peak value matches amplitude when active
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DStep, PeakApproachesAmplitudeWhenActive)
{
    TimedGaussStimulus2D tgs(makeCP("s", 50, 50),
        makeTGSP2(3.0, 20.0, 25.0, 25.0, {{0.0, 100.0}}, false, false));
    tgs.init();
    tgs.step(1.0, 1.0);
    const auto out = tgs.getComponent("output");
    EXPECT_NEAR(*std::ranges::max_element(out), 20.0, 1.0);
}

// ---------------------------------------------------------------------------
// setParameters / getParameters
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DParameters, GetParametersRoundtrip)
{
    const auto p = makeTGSP2(4.0, 12.0, 20.0, 30.0, {{0.0, 5.0}}, false, false);
    TimedGaussStimulus2D tgs(makeCP("s"), p);
    EXPECT_EQ(tgs.getParameters(), p);
}

TEST(TimedGaussStimulus2DParameters, SetParametersUpdatesPattern)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{0.0, 100.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);
    const auto before = tgs.getComponent("output");

    TimedGaussStimulus2DParameters newP{ 5.0, 15.0, 10.0, 10.0, {{0.0, 100.0}}, true, false };
    tgs.setParameters(newP);
    tgs.step(1.0, 1.0);
    const auto after = tgs.getComponent("output");

    EXPECT_NE(before, after);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DClone, CloneProducesSameOutput)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{0.0, 10.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<TimedGaussStimulus2D>(tgs.clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(1.0, 1.0);

    EXPECT_EQ(tgs.getComponent("output"), cloned->getComponent("output"));
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DToString, NonEmpty)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2());
    EXPECT_FALSE(tgs.toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulus2DEdgeCases, PositionYAtYMaxThrows)
{
    EXPECT_THROW(TimedGaussStimulus2D(makeCP("s", 50, 50),
        makeTGSP2(5.0, 15.0, 25.0, 50.0)), Exception);
}

TEST(TimedGaussStimulus2DEdgeCases, ZeroAmplitudeOutputAllZerosWhenActive)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 0.0, 25.0, 25.0, {{0.0, 100.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(TimedGaussStimulus2DEdgeCases, OutputNoNaNOrInfWhenActive)
{
    TimedGaussStimulus2D tgs(makeCP("s"), makeTGSP2(5.0, 15.0, 25.0, 25.0, {{0.0, 100.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}
