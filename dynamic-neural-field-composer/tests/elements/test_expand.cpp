#include <gtest/gtest.h>
#include <memory>

#include "elements/expand.h"
#include "elements/gauss_stimulus.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Expand> makeExpand(const std::string& name,
    const int inSize, const int outX, const int outY, const ProjectionAxis profileAxis)
{
    const ElementDimensions inDim{ inSize, 1.0 };
    const ExpandParameters ep{ profileAxis, inDim };
    const ElementCommonParameters common{ name, ElementDimensions{ outX, outY, 1.0, 1.0 } };
    return std::make_shared<Expand>(common, ep);
}

// position 0.0 keeps the Gaussian centre in range even for tiny test fields
// (the actual profile is irrelevant — it is overwritten before stepping).
static std::shared_ptr<GaussStimulus> makeSource(const std::string& name, const int size)
{
    const GaussStimulusParameters gp{ 1.0, 1.0, 0.0 };
    const ElementCommonParameters cp{ name, ElementDimensions{ size, 1.0 } };
    return std::make_shared<GaussStimulus>(cp, gp);
}

// Connect a known 1D profile to an Expand, step, and return the y-major 2D output.
static std::vector<double> expandVia(const std::shared_ptr<Expand>& expand,
    const std::vector<double>& profile)
{
    const auto source = makeSource("src", static_cast<int>(profile.size()));
    source->init();
    expand->addInput(source);
    expand->init();

    auto* out = source->getComponentPtr("output");
    std::copy(profile.begin(), profile.end(), out->begin());

    expand->step(0.0, 1.0);
    return expand->getComponent("output");
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(ExpandTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeExpand("ex", 50, 50, 40, ProjectionAxis::X));
}

TEST(ExpandTest, HasCorrectLabel)
{
    const auto ex = makeExpand("ex", 50, 50, 40, ProjectionAxis::X);
    EXPECT_EQ(ex->getLabel(), ElementLabel::EXPAND);
}

TEST(ExpandTest, BufferSizesAfterInit)
{
    const auto ex = makeExpand("ex", 8, 8, 6, ProjectionAxis::X);
    ex->init();
    EXPECT_EQ(static_cast<int>(ex->getComponent("input").size()), 8);
    EXPECT_EQ(static_cast<int>(ex->getComponent("output").size()), 8 * 6);
}

// ---------------------------------------------------------------------------
// Numerical correctness
// ---------------------------------------------------------------------------

TEST(ExpandTest, BroadcastProfileAlongX)
{
    // Output 3x2, profile along X (size 3) repeated for each of the 2 rows.
    const auto ex = makeExpand("ex", 3, 3, 2, ProjectionAxis::X);
    const auto out = expandVia(ex, { 10.0, 20.0, 30.0 });
    ASSERT_EQ(out.size(), 6u);
    // y-major: row0 = [10 20 30], row1 = [10 20 30]
    const std::vector<double> expected{ 10, 20, 30, 10, 20, 30 };
    for (size_t i = 0; i < expected.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], expected[i]);
}

TEST(ExpandTest, BroadcastProfileAlongY)
{
    // Output 3x2, profile along Y (size 2) repeated across the 3 columns.
    const auto ex = makeExpand("ex", 2, 3, 2, ProjectionAxis::Y);
    const auto out = expandVia(ex, { 7.0, 8.0 });
    ASSERT_EQ(out.size(), 6u);
    // y-major: row0 (y=0) all 7, row1 (y=1) all 8
    const std::vector<double> expected{ 7, 7, 7, 8, 8, 8 };
    for (size_t i = 0; i < expected.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], expected[i]);
}

// ---------------------------------------------------------------------------
// addInput / single-input
// ---------------------------------------------------------------------------

TEST(ExpandTest, AddInputResizesInputBuffer)
{
    const auto ex = makeExpand("ex", 4, 25, 25, ProjectionAxis::X);
    const auto source = makeSource("src", 25);
    source->init();
    ex->addInput(source);
    EXPECT_EQ(static_cast<int>(ex->getComponent("input").size()), 25);
    EXPECT_EQ(ex->getParameters().inputDimensions.size, 25);
}

TEST(ExpandTest, SecondInputIsRejected)
{
    const auto ex = makeExpand("ex", 3, 3, 2, ProjectionAxis::X);
    const auto first  = makeSource("first", 3);
    const auto second = makeSource("second", 7);
    first->init();
    second->init();
    ex->addInput(first);
    ex->addInput(second);
    EXPECT_EQ(ex->getInputs().size(), 1u);
    EXPECT_EQ(static_cast<int>(ex->getComponent("input").size()), 3);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(ExpandTest, CloneProducesIdenticalOutput)
{
    const auto ex = makeExpand("ex", 3, 3, 2, ProjectionAxis::X);
    const auto out = expandVia(ex, { 10.0, 20.0, 30.0 });

    const auto clone = std::dynamic_pointer_cast<Expand>(ex->clone());
    ASSERT_NE(clone, nullptr);
    clone->removeInputs();
    const auto cloneOut = expandVia(clone, { 10.0, 20.0, 30.0 });

    ASSERT_EQ(out.size(), cloneOut.size());
    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], cloneOut[i]);
}

TEST(ExpandTest, ToStringReturnsNonEmpty)
{
    const auto ex = makeExpand("ex", 50, 50, 40, ProjectionAxis::X);
    EXPECT_FALSE(ex->toString().empty());
}
