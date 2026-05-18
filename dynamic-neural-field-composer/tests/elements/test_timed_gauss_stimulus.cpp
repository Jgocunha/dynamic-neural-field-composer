#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <memory>

#include "elements/timed_gauss_stimulus.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int size = 100)
{
    return ElementCommonParameters{ name, size };
}

static TimedGaussStimulusParameters makeTGSP(double width = 5.0, double amp = 15.0,
    double pos = 50.0,
    std::vector<std::pair<double,double>> onTimes = {{0.0, 10.0}},
    bool circ = true, bool norm = false)
{
    return TimedGaussStimulusParameters{ width, amp, pos, std::move(onTimes), circ, norm };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(TimedGaussStimulus(makeCP("s"), makeTGSP()));
}

TEST(TimedGaussStimulusConstruction, NegativePositionThrows)
{
    EXPECT_THROW(TimedGaussStimulus(makeCP("s"),
        makeTGSP(5.0, 15.0, -1.0)), Exception);
}

TEST(TimedGaussStimulusConstruction, PositionAtXMaxThrows)
{
    EXPECT_THROW(TimedGaussStimulus(makeCP("s", 100),
        makeTGSP(5.0, 15.0, 100.0)), Exception);
}

TEST(TimedGaussStimulusConstruction, LabelIsTimedGaussStimulus)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP());
    EXPECT_EQ(tgs.getLabel(), ElementLabel::TIMED_GAUSS_STIMULUS);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusInit, OutputInitiallyZero)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP());
    tgs.init();
    const auto out = tgs.getComponent("output");
    for (double v : out)
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulusInit, OutputComponentHasCorrectSize)
{
    TimedGaussStimulus tgs(makeCP("s", 80), makeTGSP(5.0, 15.0, 40.0));
    tgs.init();
    EXPECT_EQ(static_cast<int>(tgs.getComponent("output").size()), 80);
}

// ---------------------------------------------------------------------------
// step() — time gating
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusStep, OutputIsNonZeroDuringOnInterval)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(3.0, 1.0);  // t=3 is inside [1, 5]
    const auto out = tgs.getComponent("output");
    const double maxVal = *std::ranges::max_element(out);
    EXPECT_GT(maxVal, 0.0);
}

TEST(TimedGaussStimulusStep, OutputIsZeroOutsideOnInterval)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(0.0, 1.0);  // t=0 is before [1, 5]
    const auto out = tgs.getComponent("output");
    for (double v : out)
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulusStep, OutputIsZeroAfterOnInterval)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{1.0, 5.0}}));
    tgs.init();
    tgs.step(6.0, 1.0);  // t=6 is after [1, 5]
    const auto out = tgs.getComponent("output");
    for (double v : out)
        EXPECT_EQ(v, 0.0);
}

TEST(TimedGaussStimulusStep, OutputActiveAtIntervalBoundaries)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{2.0, 4.0}}));
    tgs.init();

    tgs.step(2.0, 1.0);
    const auto atStart = tgs.getComponent("output");
    EXPECT_GT(*std::ranges::max_element(atStart), 0.0);

    tgs.step(4.0, 1.0);
    const auto atEnd = tgs.getComponent("output");
    EXPECT_GT(*std::ranges::max_element(atEnd), 0.0);
}

TEST(TimedGaussStimulusStep, MultipleIntervalsActivateCorrectly)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{1.0, 2.0}, {5.0, 6.0}}));
    tgs.init();

    tgs.step(1.5, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }

    tgs.step(3.0, 1.0);  // between intervals
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);

    tgs.step(5.5, 1.0);
    { const auto out = tgs.getComponent("output"); EXPECT_GT(*std::ranges::max_element(out), 0.0); }
}

TEST(TimedGaussStimulusStep, EmptyOnTimesAlwaysOff)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {}));
    tgs.init();
    tgs.step(100.0, 1.0);
    for (double v : tgs.getComponent("output"))
        EXPECT_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// Peak value matches amplitude when active
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusStep, PeakApproachesAmplitudeWhenActive)
{
    TimedGaussStimulus tgs(makeCP("s", 100),
        makeTGSP(3.0, 20.0, 50.0, {{0.0, 100.0}}, false, false));
    tgs.init();
    tgs.step(1.0, 1.0);
    const auto out = tgs.getComponent("output");
    EXPECT_NEAR(*std::ranges::max_element(out), 20.0, 1.0);
}

// ---------------------------------------------------------------------------
// setParameters / getParameters
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusParameters, GetParametersRoundtrip)
{
    const auto p = makeTGSP(4.0, 12.0, 30.0, {{0.0, 5.0}}, false, false);
    TimedGaussStimulus tgs(makeCP("s"), p);
    EXPECT_EQ(tgs.getParameters(), p);
}

TEST(TimedGaussStimulusParameters, SetParametersUpdatesPattern)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{0.0, 100.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);
    const auto before = tgs.getComponent("output");

    TimedGaussStimulusParameters newP{ 5.0, 15.0, 20.0, {{0.0, 100.0}}, true, false };
    tgs.setParameters(newP);
    tgs.step(1.0, 1.0);
    const auto after = tgs.getComponent("output");

    EXPECT_NE(before, after);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusClone, CloneProducesSameOutput)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP(5.0, 15.0, 50.0, {{0.0, 10.0}}));
    tgs.init();
    tgs.step(1.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<TimedGaussStimulus>(tgs.clone());
    ASSERT_NE(cloned, nullptr);
    cloned->step(1.0, 1.0);

    EXPECT_EQ(tgs.getComponent("output"), cloned->getComponent("output"));
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(TimedGaussStimulusToString, NonEmpty)
{
    TimedGaussStimulus tgs(makeCP("s"), makeTGSP());
    EXPECT_FALSE(tgs.toString().empty());
}
