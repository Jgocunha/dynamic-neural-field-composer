#include <gtest/gtest.h>
#include <algorithm>
#include <memory>

#include "elements/boost_stimulus.h"
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

static BoostStimulusParameters makeBSP(const double amplitude = 5.0, const bool isActive = true)
{
    return BoostStimulusParameters{ amplitude, isActive };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(BoostStimulusConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(BoostStimulus(makeCP("b"), makeBSP()));
}

TEST(BoostStimulusConstruction, LabelIsBoostStimulus)
{
    BoostStimulus bs(makeCP("b"), makeBSP());
    EXPECT_EQ(bs.getLabel(), ElementLabel::BOOST_STIMULUS);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(BoostStimulusInit, OutputComponentHasCorrectSize)
{
    BoostStimulus bs(makeCP("b", 80), makeBSP());
    bs.init();
    const auto out = bs.getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 80);
}

TEST(BoostStimulusInit, ActiveOutputIsUniformAmplitude)
{
    BoostStimulus bs(makeCP("b", 100), makeBSP(7.0, true));
    bs.init();
    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 7.0);
}

TEST(BoostStimulusInit, InactiveOutputIsAllZeros)
{
    BoostStimulus bs(makeCP("b", 100), makeBSP(7.0, false));
    bs.init();
    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BoostStimulusInit, ZeroAmplitudeOutputIsAllZeros)
{
    BoostStimulus bs(makeCP("b"), makeBSP(0.0, true));
    bs.init();
    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BoostStimulusInit, NegativeAmplitudeIsAllowed)
{
    BoostStimulus bs(makeCP("b"), makeBSP(-3.0, true));
    bs.init();
    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, -3.0);
}

// ---------------------------------------------------------------------------
// step() — toggling isActive mid-simulation
// ---------------------------------------------------------------------------

TEST(BoostStimulusStep, OutputUnchangedWhenActiveAndNotToggled)
{
    BoostStimulus bs(makeCP("b"), makeBSP(5.0, true));
    bs.init();
    const auto before = bs.getComponent("output");
    bs.step(1.0, 0.1);
    const auto after = bs.getComponent("output");
    EXPECT_EQ(before, after);
}

TEST(BoostStimulusStep, TogglingOffViaSetParametersZerosOutput)
{
    BoostStimulus bs(makeCP("b"), makeBSP(5.0, true));
    bs.init();

    auto params = bs.getParameters();
    params.isActive = false;
    bs.setParameters(params);

    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BoostStimulusStep, TogglingOnViaSetParametersRestoresAmplitude)
{
    BoostStimulus bs(makeCP("b"), makeBSP(5.0, false));
    bs.init();

    auto params = bs.getParameters();
    params.isActive = true;
    bs.setParameters(params);

    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 5.0);
}

TEST(BoostStimulusStep, StepReflectsActiveState)
{
    BoostStimulus bs(makeCP("b"), makeBSP(4.0, false));
    bs.init();
    bs.step(0.0, 0.1);
    const auto outOff = bs.getComponent("output");
    for (const double v : outOff)
        EXPECT_DOUBLE_EQ(v, 0.0);

    bs.setParameters(makeBSP(4.0, true));
    bs.step(0.1, 0.1);
    const auto outOn = bs.getComponent("output");
    for (const double v : outOn)
        EXPECT_DOUBLE_EQ(v, 4.0);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(BoostStimulusParameters, GetParametersRoundtrip)
{
    const auto bsp = makeBSP(8.0, false);
    const BoostStimulus bs(makeCP("b"), bsp);
    EXPECT_EQ(bs.getParameters(), bsp);
}

TEST(BoostStimulusParameters, SetAmplitudeUpdatesOutput)
{
    BoostStimulus bs(makeCP("b"), makeBSP(5.0, true));
    bs.init();

    bs.setParameters(makeBSP(20.0, true));
    const auto out = bs.getComponent("output");
    for (const double v : out)
        EXPECT_DOUBLE_EQ(v, 20.0);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(BoostStimulusClone, CloneHasSameParameters)
{
    const auto bsp = makeBSP(3.0, false);
    BoostStimulus bs(makeCP("b"), bsp);
    bs.init();
    const auto cloned = bs.clone();
    const auto clonedBS = std::dynamic_pointer_cast<BoostStimulus>(cloned);
    ASSERT_NE(clonedBS, nullptr);
    EXPECT_EQ(clonedBS->getParameters(), bs.getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(BoostStimulusToString, NonEmpty)
{
    const BoostStimulus bs(makeCP("b"), makeBSP());
    EXPECT_FALSE(bs.toString().empty());
}

// ---------------------------------------------------------------------------
// Element base: name, size, label
// ---------------------------------------------------------------------------

TEST(BoostStimulusElement, UniqueNameMatchesConstruction)
{
    const BoostStimulus bs(makeCP("my-boost"), makeBSP());
    EXPECT_EQ(bs.getUniqueName(), "my-boost");
}

TEST(BoostStimulusElement, SizeMatchesDimension)
{
    const BoostStimulus bs(makeCP("b", 60), makeBSP());
    EXPECT_EQ(bs.getSize(), 60);
}
