#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <numeric>

#include "elements/resize.h"
#include "elements/gauss_stimulus.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, const int inputSize = 100)
{
    return ElementCommonParameters{ name, ElementDimensions{ inputSize, 1.0 } };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(ResizeConstruction, LabelIsResize)
{
    const Resize r(makeCP("r"), ResizeParameters(50));
    EXPECT_EQ(r.getLabel(), ElementLabel::RESIZE);
}

TEST(ResizeConstruction, InputComponentSizeMatchesCommonParameters)
{
    const Resize r(makeCP("r", 100), ResizeParameters(50));
    const auto inp = r.getComponent("input");
    EXPECT_EQ(static_cast<int>(inp.size()), 100);
}

TEST(ResizeConstruction, OutputComponentSizeMatchesOutputSize)
{
    Resize r(makeCP("r", 100), ResizeParameters(50));
    r.init();
    const auto out = r.getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 50);
}

// ---------------------------------------------------------------------------
// step() output size
// ---------------------------------------------------------------------------

TEST(ResizeStep, OutputSizeMatchesOutputParameters)
{
    auto upstream = std::make_shared<GaussStimulus>(
        makeCP("gs", 100),
        GaussStimulusParameters{ 5.0, 10.0, 50.0, false, false });
    upstream->init();

    auto resize = std::make_shared<Resize>(makeCP("r", 100), ResizeParameters(60));
    resize->init();
    resize->addInput(upstream, "output");

    for (int i = 0; i < 5; ++i)
        resize->step(0.0, 1.0);

    const auto out = resize->getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 60);
}

// ---------------------------------------------------------------------------
// Resample correctness — constant input
// ---------------------------------------------------------------------------

TEST(ResizeStep, ConstantInputProducesConstantOutput)
{
    auto upstream = std::make_shared<GaussStimulus>(
        makeCP("gs", 100),
        GaussStimulusParameters{ 1000.0, 1.0, 50.0, false, false });
    upstream->init();

    // After many steps the output of the Gauss stimulus converges to ~amplitude
    for (int i = 0; i < 10; ++i)
        upstream->step(0.0, 1.0);

    auto resize = std::make_shared<Resize>(makeCP("r", 100), ResizeParameters(40));
    resize->init();
    resize->addInput(upstream, "output");
    resize->step(0.0, 1.0);

    const auto out = resize->getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 40);

    // All output values should be equal (resampled from a constant input)
    for (size_t i = 1; i < out.size(); ++i)
        EXPECT_NEAR(out[i], out[0], 1e-6);
}

// ---------------------------------------------------------------------------
// setParameters round-trip
// ---------------------------------------------------------------------------

TEST(ResizeSetParameters, OutputSizeChanges)
{
    Resize r(makeCP("r", 100), ResizeParameters(50));
    r.init();
    EXPECT_EQ(static_cast<int>(r.getComponent("output").size()), 50);

    r.setParameters(ResizeParameters(75, 0.5));
    EXPECT_EQ(static_cast<int>(r.getComponent("output").size()), 75);
    EXPECT_DOUBLE_EQ(r.getParameters().outputStep, 0.5);
}

// ---------------------------------------------------------------------------
// clone()
// ---------------------------------------------------------------------------

TEST(ResizeClone, CloneHasSameOutputSize)
{
    auto r = std::make_shared<Resize>(makeCP("r", 100), ResizeParameters(60));
    r->init();
    const auto cloned = r->clone();
    EXPECT_EQ(static_cast<int>(cloned->getComponent("output").size()), 60);
}

TEST(ResizeClone, CloneLabelIsResize)
{
    auto r = std::make_shared<Resize>(makeCP("r", 100), ResizeParameters(60));
    r->init();
    const auto cloned = r->clone();
    EXPECT_EQ(cloned->getLabel(), ElementLabel::RESIZE);
}
