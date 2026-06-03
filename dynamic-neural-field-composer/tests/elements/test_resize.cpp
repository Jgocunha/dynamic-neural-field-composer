#include <gtest/gtest.h>
#include <memory>

#include "elements/resize.h"
#include "elements/gauss_stimulus.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Resize> makeResize(const std::string& name,
    const int inSize, const int outSize,
    const InterpolationMethod method = InterpolationMethod::LINEAR)
{
    const ElementDimensions inDim{ inSize, 1.0 };
    const ResizeParameters rp{ method, inDim };
    const ElementCommonParameters cp{ name, ElementDimensions{ outSize, 1.0 } };
    return std::make_shared<Resize>(cp, rp);
}

// A generic source of a given size whose "output" we overwrite with a known vector.
// position 0.0 keeps the Gaussian centre in range even for tiny test fields (the
// actual profile is irrelevant — it is overwritten before stepping).
static std::shared_ptr<GaussStimulus> makeSource(const std::string& name, const int size)
{
    const GaussStimulusParameters gp{ 1.0, 1.0, 0.0 };
    const ElementCommonParameters cp{ name, ElementDimensions{ size, 1.0 } };
    return std::make_shared<GaussStimulus>(cp, gp);
}

// Connect a source of the given values to a Resize, step it, and return the output.
static std::vector<double> resampleVia(const std::shared_ptr<Resize>& resize,
    const std::vector<double>& input)
{
    const auto source = makeSource("src", static_cast<int>(input.size()));
    source->init();
    resize->addInput(source);
    resize->init();

    // Overwrite the source output in place with the known input vector. Writing in
    // place (rather than reassigning) keeps the buffer address stable; step() builds
    // the input cache lazily on first call.
    auto* out = source->getComponentPtr("output");
    std::copy(input.begin(), input.end(), out->begin());

    resize->step(0.0, 1.0);
    return resize->getComponent("output");
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(ResizeTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeResize("rz", 50, 100));
}

TEST(ResizeTest, HasCorrectLabel)
{
    const auto rz = makeResize("rz", 50, 100);
    EXPECT_EQ(rz->getLabel(), ElementLabel::RESIZE);
}

TEST(ResizeTest, OutputSizeMatchesDeclaredDimension)
{
    const auto rz = makeResize("rz", 50, 80);
    rz->init();
    EXPECT_EQ(static_cast<int>(rz->getComponent("output").size()), 80);
}

TEST(ResizeTest, InputSizeMatchesInputDimensions)
{
    const auto rz = makeResize("rz", 37, 80);
    rz->init();
    EXPECT_EQ(static_cast<int>(rz->getComponent("input").size()), 37);
}

// ---------------------------------------------------------------------------
// addInput resizes the input buffer to the source size
// ---------------------------------------------------------------------------

TEST(ResizeTest, AddInputResizesInputBuffer)
{
    const auto rz = makeResize("rz", 10, 80); // declared input size 10 ...
    const auto source = makeSource("src", 25); // ... but source is size 25
    source->init();
    rz->addInput(source);
    EXPECT_EQ(static_cast<int>(rz->getComponent("input").size()), 25);
    EXPECT_EQ(rz->getParameters().inputDimensions.size, 25);
}

// Regression: Resize is single-input. A second input must be rejected so that the
// resized "input" buffer cannot be overrun by a larger second source in step().
TEST(ResizeTest, SecondInputIsRejected)
{
    const auto rz = makeResize("rz", 25, 80);
    const auto first  = makeSource("first", 25);
    const auto second = makeSource("second", 40);
    first->init();
    second->init();

    rz->addInput(first);
    rz->addInput(second); // must be refused

    EXPECT_EQ(rz->getInputs().size(), 1u);
    EXPECT_EQ(static_cast<int>(rz->getComponent("input").size()), 25);

    // step() must not read/write out of bounds and must reflect only the first source.
    rz->init();
    auto* out = first->getComponentPtr("output");
    std::fill(out->begin(), out->end(), 1.0);
    EXPECT_NO_THROW(rz->step(0.0, 1.0));
    for (const double v : rz->getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 1.0); // resampling a constant field yields the same constant
}

// ---------------------------------------------------------------------------
// Numerical correctness
// ---------------------------------------------------------------------------

TEST(ResizeTest, IdentityWhenSizesEqual)
{
    const auto rz = makeResize("rz", 4, 4, InterpolationMethod::LINEAR);
    const std::vector<double> in{ 1.0, 2.0, 3.0, 4.0 };
    const auto out = resampleVia(rz, in);
    ASSERT_EQ(out.size(), in.size());
    for (size_t i = 0; i < in.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], in[i]);
}

TEST(ResizeTest, LinearUpsampleOfRamp)
{
    // Ramp input 0..3 (N=4) upsampled to M=7.
    // pos_i = i * (N-1)/(M-1) = i * 3/6 = 0.5 i; for a ramp, value == position.
    const auto rz = makeResize("rz", 4, 7, InterpolationMethod::LINEAR);
    const std::vector<double> in{ 0.0, 1.0, 2.0, 3.0 };
    const auto out = resampleVia(rz, in);
    ASSERT_EQ(out.size(), 7u);
    for (int i = 0; i < 7; ++i)
        EXPECT_NEAR(out[i], 0.5 * i, 1e-9);
}

TEST(ResizeTest, LinearDownsampleEndpointsPreserved)
{
    // Endpoints are always preserved by the linear resampler.
    const auto rz = makeResize("rz", 9, 5, InterpolationMethod::LINEAR);
    std::vector<double> in(9);
    for (int i = 0; i < 9; ++i) in[i] = static_cast<double>(i); // ramp 0..8
    const auto out = resampleVia(rz, in);
    ASSERT_EQ(out.size(), 5u);
    // pos_i = i * 8/4 = 2 i -> ramp value 2 i
    for (int i = 0; i < 5; ++i)
        EXPECT_NEAR(out[i], 2.0 * i, 1e-9);
}

TEST(ResizeTest, NearestNeighbourPicksClosestSample)
{
    // N=3 -> M=5, pos_i = i * 2/4 = 0.5 i; round(0.5 i) selects nearest index.
    const auto rz = makeResize("rz", 3, 5, InterpolationMethod::NEAREST);
    const std::vector<double> in{ 10.0, 20.0, 30.0 };
    const auto out = resampleVia(rz, in);
    ASSERT_EQ(out.size(), 5u);
    // pos: 0, 0.5, 1.0, 1.5, 2.0 -> round: 0, 0(.5->0 via std::round? 0.5 rounds to 1)
    // std::round(0.5) = 1, std::round(1.5) = 2
    EXPECT_DOUBLE_EQ(out[0], in[0]); // pos 0 -> idx 0
    EXPECT_DOUBLE_EQ(out[2], in[1]); // pos 1.0 -> idx 1
    EXPECT_DOUBLE_EQ(out[4], in[2]); // pos 2.0 -> idx 2
}

TEST(ResizeTest, CubicReproducesSampleValuesAtAlignedPositions)
{
    // N=5 -> M=9, pos_i = i * 4/8 = 0.5 i. At even output indices the position is an
    // integer, so the output must equal the original sample exactly (any interpolation
    // passes through the sample points). Odd indices fall between samples; for the
    // interior intervals Catmull-Rom reproduces a linear ramp, but the boundary
    // intervals use clamped endpoints and are not exactly linear, so we don't assert
    // those.
    const auto rz = makeResize("rz", 5, 9, InterpolationMethod::CUBIC);
    std::vector<double> in(5);
    for (int i = 0; i < 5; ++i) in[i] = static_cast<double>(i);
    const auto out = resampleVia(rz, in);
    ASSERT_EQ(out.size(), 9u);
    for (int i = 0; i < 9; i += 2)              // aligned sample points
        EXPECT_NEAR(out[i], 0.5 * i, 1e-9);
    // interior midpoint (between in[1] and in[2]) is exact for Catmull-Rom on a ramp
    EXPECT_NEAR(out[3], 1.5, 1e-9);
    EXPECT_NEAR(out[5], 2.5, 1e-9);
}

// ---------------------------------------------------------------------------
// Parameter update
// ---------------------------------------------------------------------------

TEST(ResizeTest, SetParametersChangesMethod)
{
    const auto rz = makeResize("rz", 3, 5, InterpolationMethod::LINEAR);
    ResizeParameters p = rz->getParameters();
    p.method = InterpolationMethod::NEAREST;
    rz->setParameters(p);
    EXPECT_EQ(rz->getParameters().method, InterpolationMethod::NEAREST);
}

// ---------------------------------------------------------------------------
// toString / clone
// ---------------------------------------------------------------------------

TEST(ResizeTest, ToStringReturnsNonEmpty)
{
    const auto rz = makeResize("rz", 50, 100);
    EXPECT_FALSE(rz->toString().empty());
}

TEST(ResizeTest, CloneProducesIdenticalOutput)
{
    const auto rz = makeResize("rz", 4, 7, InterpolationMethod::LINEAR);
    const std::vector<double> in{ 0.0, 1.0, 2.0, 3.0 };
    const auto out = resampleVia(rz, in);

    const auto clone = std::dynamic_pointer_cast<Resize>(rz->clone());
    ASSERT_NE(clone, nullptr);
    // The clone is a copy and already carries the original's input connection;
    // clear it so resampleVia wires a single fresh source (otherwise two sources
    // would be summed, doubling the output).
    clone->removeInputs();
    const auto cloneOut = resampleVia(clone, in);

    ASSERT_EQ(out.size(), cloneOut.size());
    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], cloneOut[i]);
}

// ---------------------------------------------------------------------------
// Parameters equality / toString
// ---------------------------------------------------------------------------

TEST(ResizeParametersTest, EqualParametersCompareEqual)
{
    const ElementDimensions dim{ 50, 1.0 };
    const ResizeParameters a{ InterpolationMethod::LINEAR, dim };
    const ResizeParameters b{ InterpolationMethod::LINEAR, dim };
    EXPECT_EQ(a, b);
}

TEST(ResizeParametersTest, DifferentMethodComparesNotEqual)
{
    const ElementDimensions dim{ 50, 1.0 };
    const ResizeParameters a{ InterpolationMethod::LINEAR,  dim };
    const ResizeParameters b{ InterpolationMethod::NEAREST, dim };
    EXPECT_NE(a, b);
}
