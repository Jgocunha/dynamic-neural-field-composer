#include <gtest/gtest.h>
#include <memory>
#include <cmath>

#include "elements/boost_stimulus_2d.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int xmax = 50, int ymax = 50)
{
    return ElementCommonParameters{ name, ElementDimensions{ xmax, ymax, 1.0, 1.0 } };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(BoostStimulus2D(makeCP("b"), BoostStimulus2DParameters{}));
}

TEST(BoostStimulus2DConstruction, LabelIsBoostStimulus2D)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{});
    EXPECT_EQ(bs.getLabel(), ElementLabel::BOOST_STIMULUS_2D);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DInit, OutputFillsWithAmplitudeWhenActive)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 7.0, true });
    bs.init();
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 7.0);
}

TEST(BoostStimulus2DInit, OutputIsZeroWhenInactive)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 7.0, false });
    bs.init();
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BoostStimulus2DInit, OutputHasCorrectSize)
{
    BoostStimulus2D bs(makeCP("b", 30, 20), BoostStimulus2DParameters{});
    bs.init();
    EXPECT_EQ(static_cast<int>(bs.getComponent("output").size()), 30 * 20);
}

// ---------------------------------------------------------------------------
// step()
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DStep, OutputMatchesAmplitudeDuringStep)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 3.5, true });
    bs.init();
    bs.step(0.0, 1.0);
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 3.5);
}

TEST(BoostStimulus2DStep, InactiveOutputIsZero)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 3.5, false });
    bs.init();
    bs.step(0.0, 1.0);
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// setParameters / getParameters
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DParameters, GetParametersRoundtrip)
{
    const BoostStimulus2DParameters p{ 8.0, false };
    BoostStimulus2D bs(makeCP("b"), p);
    EXPECT_EQ(bs.getParameters(), p);
}

TEST(BoostStimulus2DParameters, SetParametersUpdatesOutput)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 5.0, true });
    bs.init();

    bs.setParameters(BoostStimulus2DParameters{ 10.0, true });
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 10.0);
}

TEST(BoostStimulus2DParameters, SetActiveToFalseZerosOutput)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 5.0, true });
    bs.init();
    bs.setParameters(BoostStimulus2DParameters{ 5.0, false });
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DClone, CloneProducesSameOutput)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 4.0, true });
    bs.init();

    const auto cloned = std::dynamic_pointer_cast<BoostStimulus2D>(bs.clone());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(bs.getComponent("output"), cloned->getComponent("output"));
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DToString, NonEmpty)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{});
    EXPECT_FALSE(bs.toString().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST(BoostStimulus2DEdgeCases, ZeroAmplitudeActiveOutputAllZeros)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 0.0, true });
    bs.init();
    for (double v : bs.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(BoostStimulus2DEdgeCases, OutputNoNaNOrInf)
{
    BoostStimulus2D bs(makeCP("b"), BoostStimulus2DParameters{ 5.0, true });
    bs.init();
    bs.step(0.0, 1.0);
    for (double v : bs.getComponent("output"))
        EXPECT_TRUE(std::isfinite(v));
}
