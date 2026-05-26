#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <cmath>

#include "elements/neural_field_2d.h"
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

TEST(NeuralField2DEdgeCases, HighRestingLevelRemainsFinite)
{
    NeuralField2D nf(makeCP("nf", 4, 4), makeNFP(25.0, 10.0));
    nf.init();
    for (int i = 0; i < 100; ++i)
        nf.step(static_cast<double>(i), 1.0);
    for (double v : nf.getComponent("activation"))
        EXPECT_TRUE(std::isfinite(v));
}
