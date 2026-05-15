#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
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

// Build a Resize with a constant input signal, run n steps.
// Creates a truly constant input by directly filling the GaussStimulus output.
static std::vector<double> runResize(int inputSize, int outputSize,
    ResizeInterpolation interp, int steps = 5)
{
    GaussStimulusParameters gsp{ 1.0, 1.0, static_cast<double>(inputSize / 2), false, false };
    auto upstream = std::make_shared<GaussStimulus>(makeCP("gs", inputSize), gsp);
    upstream->init();
    
    // Fill the upstream output with a constant value for a truly constant input
    // Use getComponentPtr() to modify the element's internal buffer rather than a copy.
    std::vector<double>* upstreamOutputPtr = upstream->getComponentPtr("output");
    std::ranges::fill(*upstreamOutputPtr, 0.5);

    const auto resize = std::make_shared<Resize>(
        makeCP("r", inputSize), ResizeParameters(outputSize, 1.0, interp));
    resize->init();
    resize->addInput(upstream, "output");
    for (int i = 0; i < steps; ++i)
        resize->step(0.0, 1.0);

    return resize->getComponent("output");
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(ResizeConstruction, LabelIsResize)
{
    Resize r(makeCP("r"), ResizeParameters(50));
    EXPECT_EQ(r.getLabel(), ElementLabel::RESIZE);
}

TEST(ResizeConstruction, InputComponentSizeMatchesCommonParameters)
{
    Resize r(makeCP("r", 100), ResizeParameters(50));
    const auto inp = r.getComponent("input");
    EXPECT_EQ(static_cast<int>(inp.size()), 100);
}

TEST(ResizeConstruction, OutputComponentSizeMatchesOutputSize)
{
    Resize r(makeCP("r", 100), ResizeParameters(50));
    r.init();
    EXPECT_EQ(static_cast<int>(r.getComponent("output").size()), 50);
}

TEST(ResizeConstruction, DefaultInterpolationIsLinear)
{
    Resize r(makeCP("r", 100), ResizeParameters(50));
    EXPECT_EQ(r.getParameters().interpolation, ResizeInterpolation::LINEAR);
}

// ---------------------------------------------------------------------------
// step() — output dimensions
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

    EXPECT_EQ(static_cast<int>(resize->getComponent("output").size()), 60);
}

TEST(ResizeStep, UpsampleOutputSizeCorrect)
{
    const auto out = runResize(50, 150, ResizeInterpolation::LINEAR);
    EXPECT_EQ(static_cast<int>(out.size()), 150);
}

TEST(ResizeStep, IdenticalSizesOutputEqualsInput)
{
    // When input size == output size the signal should be copied unchanged.
    GaussStimulusParameters gsp{ 5.0, 10.0, 50.0, false, false };
    auto upstream = std::make_shared<GaussStimulus>(makeCP("gs", 100), gsp);
    upstream->init();
    for (int i = 0; i < 5; ++i) upstream->step(0.0, 1.0);

    auto resize = std::make_shared<Resize>(makeCP("r", 100), ResizeParameters(100));
    resize->init();
    resize->addInput(upstream, "output");
    resize->step(0.0, 1.0);

    const auto refOut  = upstream->getComponent("output");
    const auto resOut  = resize->getComponent("output");
    ASSERT_EQ(refOut.size(), resOut.size());
    for (size_t i = 0; i < refOut.size(); ++i)
        EXPECT_NEAR(resOut[i], refOut[i], 1e-9);
}

// ---------------------------------------------------------------------------
// Resample correctness — constant input
// ---------------------------------------------------------------------------

TEST(ResizeStep, ConstantInputProducesConstantOutput_Linear)
{
    const auto out = runResize(100, 40, ResizeInterpolation::LINEAR);
    ASSERT_EQ(static_cast<int>(out.size()), 40);
    for (size_t i = 1; i < out.size(); ++i)
        EXPECT_NEAR(out[i], out[0], 1e-6);
}

TEST(ResizeStep, ConstantInputProducesConstantOutput_Nearest)
{
    const auto out = runResize(100, 40, ResizeInterpolation::NEAREST);
    ASSERT_EQ(static_cast<int>(out.size()), 40);
    for (size_t i = 1; i < out.size(); ++i)
        EXPECT_NEAR(out[i], out[0], 1e-6);
}

TEST(ResizeStep, ConstantInputProducesConstantOutput_Cubic)
{
    const auto out = runResize(100, 40, ResizeInterpolation::CUBIC);
    ASSERT_EQ(static_cast<int>(out.size()), 40);
    for (size_t i = 1; i < out.size(); ++i)
        EXPECT_NEAR(out[i], out[0], 1e-4);
}

// ---------------------------------------------------------------------------
// Interpolation — functional correctness
// ---------------------------------------------------------------------------

// Build a Resize directly from a known input vector without GaussStimulus.
// We inject via a GaussStimulus with a flat profile (very wide, large amp).
// Easier: directly verify math on a known synthetic signal using the math utils.
// Here we test via the element pipeline using a ramp input constructed from
// two GaussStimuli at the endpoints to produce a near-linear profile.

TEST(ResizeInterpolation, NearestPicksClosestSample)
{
    // Input: 4 positions [0, 1, 2, 3] (index values for easy verification).
    // We cannot inject arbitrary values via GaussStimulus, so we verify the
    // structural property: nearest resampling from N to N/2 must pick every
    // other sample (indices 0, 2 for a 4→2 downsample).
    // Verified indirectly: output values must be a subset of input values.
    GaussStimulusParameters gsp{ 1.0, 5.0, 2.0, false, false };
    auto upstream = std::make_shared<GaussStimulus>(makeCP("gs", 10), gsp);
    upstream->init();
    for (int i = 0; i < 10; ++i) upstream->step(0.0, 1.0);

    auto resize = std::make_shared<Resize>(
        makeCP("r", 10), ResizeParameters(5, 1.0, ResizeInterpolation::NEAREST));
    resize->init();
    resize->addInput(upstream, "output");
    resize->step(0.0, 1.0);

    const auto inp = upstream->getComponent("output");
    const auto out = resize->getComponent("output");
    ASSERT_EQ(static_cast<int>(out.size()), 5);

    // Every output value must exactly match one of the input values (nearest = no blending).
    for (const double v : out)
    {
        bool found = std::any_of(inp.begin(), inp.end(),
            [v](double x) { return std::abs(x - v) < 1e-9; });
        EXPECT_TRUE(found) << "Nearest output value " << v << " not found in input";
    }
}

TEST(ResizeInterpolation, LinearBlendsBoundingSamples)
{
    // A linear ramp input: value at position i is exactly i.
    // After linear resampling from N to M, output[j] = j * (N-1) / (M-1).
    // We cannot inject a pure ramp via GaussStimulus, so we verify that
    // linear resampling produces values strictly between the input extremes
    // when the input is monotonically increasing (guaranteed by a Gauss
    // stimulus centered at one end, so output grows from left to right).
    GaussStimulusParameters gsp{ 3.0, 10.0, 0.0, false, false };
    auto upstream = std::make_shared<GaussStimulus>(makeCP("gs", 20), gsp);
    upstream->init();
    for (int i = 0; i < 10; ++i) upstream->step(0.0, 1.0);

    const auto inp = upstream->getComponent("output");
    const double inputMin = *std::min_element(inp.begin(), inp.end());
    const double inputMax = *std::max_element(inp.begin(), inp.end());

    auto resize = std::make_shared<Resize>(
        makeCP("r", 20), ResizeParameters(40, 1.0, ResizeInterpolation::LINEAR));
    resize->init();
    resize->addInput(upstream, "output");
    resize->step(0.0, 1.0);

    const auto out = resize->getComponent("output");
    ASSERT_EQ(static_cast<int>(out.size()), 40);
    for (const double v : out)
    {
        EXPECT_GE(v, inputMin - 1e-9);
        EXPECT_LE(v, inputMax + 1e-9);
    }
}

TEST(ResizeInterpolation, CubicOutputSizeCorrect)
{
    const auto out = runResize(100, 70, ResizeInterpolation::CUBIC);
    EXPECT_EQ(static_cast<int>(out.size()), 70);
}

TEST(ResizeInterpolation, CubicValuesWithinInputRange)
{
    // Catmull-Rom may slightly overshoot at extremes; for a smooth input
    // the overshoot should be small. We verify values stay within a 5% margin
    // of the input range.
    GaussStimulusParameters gsp{ 5.0, 8.0, 50.0, false, false };
    auto upstream = std::make_shared<GaussStimulus>(makeCP("gs", 100), gsp);
    upstream->init();
    for (int i = 0; i < 10; ++i) upstream->step(0.0, 1.0);

    const auto inp = upstream->getComponent("output");
    const double inMin = *std::min_element(inp.begin(), inp.end());
    const double inMax = *std::max_element(inp.begin(), inp.end());
    const double margin = 0.05 * (inMax - inMin) + 1e-6;

    auto resize = std::make_shared<Resize>(
        makeCP("r", 100), ResizeParameters(200, 1.0, ResizeInterpolation::CUBIC));
    resize->init();
    resize->addInput(upstream, "output");
    resize->step(0.0, 1.0);

    const auto out = resize->getComponent("output");
    for (const double v : out)
    {
        EXPECT_GE(v, inMin - margin);
        EXPECT_LE(v, inMax + margin);
    }
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

TEST(ResizeSetParameters, InterpolationTypeChanges)
{
    Resize r(makeCP("r", 100), ResizeParameters(50, 1.0, ResizeInterpolation::LINEAR));
    r.init();
    EXPECT_EQ(r.getParameters().interpolation, ResizeInterpolation::LINEAR);

    r.setParameters(ResizeParameters(50, 1.0, ResizeInterpolation::CUBIC));
    EXPECT_EQ(r.getParameters().interpolation, ResizeInterpolation::CUBIC);
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

TEST(ResizeClone, ClonePreservesInterpolationType)
{
    auto r = std::make_shared<Resize>(
        makeCP("r", 100),
        ResizeParameters(60, 1.0, ResizeInterpolation::CUBIC));
    r->init();
    const auto cloned = r->clone();
    const auto clonedResize = std::dynamic_pointer_cast<Resize>(cloned);
    ASSERT_NE(clonedResize, nullptr);
    EXPECT_EQ(clonedResize->getParameters().interpolation, ResizeInterpolation::CUBIC);
}
