#include <gtest/gtest.h>
#include <memory>

#include "elements/collapse.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Collapse> makeCollapse(const std::string& name,
    const int inX, const int inY, const int outSize,
    const CompressionType compression, const ProjectionAxis keepAxis)
{
    const ElementDimensions inDim{ inX, inY, 1.0, 1.0 };
    const CollapseParameters cp{ compression, keepAxis, inDim };
    const ElementCommonParameters common{ name, ElementDimensions{ outSize, 1.0 } };
    return std::make_shared<Collapse>(common, cp);
}

// position (0,0) keeps the Gaussian centre in range even for tiny test fields
// (the actual profile is irrelevant — it is overwritten before stepping).
static std::shared_ptr<GaussStimulus2D> makeSource2D(const std::string& name,
    const int sizeX, const int sizeY)
{
    const GaussStimulus2DParameters gp{ 1.0, 1.0, 0.0, 0.0 };
    const ElementCommonParameters cp{ name, ElementDimensions{ sizeX, sizeY, 1.0, 1.0 } };
    return std::make_shared<GaussStimulus2D>(cp, gp);
}

// Connect a known y-major 2D field to a Collapse, step, and return the 1D output.
static std::vector<double> collapseVia(const std::shared_ptr<Collapse>& collapse,
    const std::vector<double>& field, const int sizeX, const int sizeY)
{
    const auto source = makeSource2D("src", sizeX, sizeY);
    source->init();
    collapse->addInput(source);
    collapse->init();

    auto* out = source->getComponentPtr("output");
    std::copy(field.begin(), field.end(), out->begin());

    collapse->step(0.0, 1.0);
    return collapse->getComponent("output");
}

// Known 3x2 (size_x=3, size_y=2) y-major field:
//   y=0: [1 2 3]
//   y=1: [4 5 6]
static std::vector<double> sampleField() { return { 1, 2, 3, 4, 5, 6 }; }

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(CollapseTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeCollapse("cl", 50, 40, 50, CompressionType::SUM, ProjectionAxis::X));
}

TEST(CollapseTest, HasCorrectLabel)
{
    const auto cl = makeCollapse("cl", 50, 40, 50, CompressionType::SUM, ProjectionAxis::X);
    EXPECT_EQ(cl->getLabel(), ElementLabel::COLLAPSE);
}

TEST(CollapseTest, BufferSizesAfterInit)
{
    const auto cl = makeCollapse("cl", 8, 6, 8, CompressionType::SUM, ProjectionAxis::X);
    cl->init();
    EXPECT_EQ(static_cast<int>(cl->getComponent("input").size()), 8 * 6);
    EXPECT_EQ(static_cast<int>(cl->getComponent("output").size()), 8);
}

// ---------------------------------------------------------------------------
// Numerical correctness — keep X (collapse over y)
// ---------------------------------------------------------------------------

TEST(CollapseTest, KeepXSum)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::SUM, ProjectionAxis::X);
    const auto out = collapseVia(cl, sampleField(), 3, 2);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_DOUBLE_EQ(out[0], 5.0);  // 1+4
    EXPECT_DOUBLE_EQ(out[1], 7.0);  // 2+5
    EXPECT_DOUBLE_EQ(out[2], 9.0);  // 3+6
}

TEST(CollapseTest, KeepXAverage)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::AVERAGE, ProjectionAxis::X);
    const auto out = collapseVia(cl, sampleField(), 3, 2);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_DOUBLE_EQ(out[0], 2.5);
    EXPECT_DOUBLE_EQ(out[1], 3.5);
    EXPECT_DOUBLE_EQ(out[2], 4.5);
}

TEST(CollapseTest, KeepXMaximum)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::MAXIMUM, ProjectionAxis::X);
    const auto out = collapseVia(cl, sampleField(), 3, 2);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_DOUBLE_EQ(out[0], 4.0);
    EXPECT_DOUBLE_EQ(out[1], 5.0);
    EXPECT_DOUBLE_EQ(out[2], 6.0);
}

TEST(CollapseTest, KeepXMinimum)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::MINIMUM, ProjectionAxis::X);
    const auto out = collapseVia(cl, sampleField(), 3, 2);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_DOUBLE_EQ(out[0], 1.0);
    EXPECT_DOUBLE_EQ(out[1], 2.0);
    EXPECT_DOUBLE_EQ(out[2], 3.0);
}

// ---------------------------------------------------------------------------
// Numerical correctness — keep Y (collapse over x)
// ---------------------------------------------------------------------------

TEST(CollapseTest, KeepYSum)
{
    const auto cl = makeCollapse("cl", 3, 2, 2, CompressionType::SUM, ProjectionAxis::Y);
    const auto out = collapseVia(cl, sampleField(), 3, 2);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_DOUBLE_EQ(out[0], 6.0);   // 1+2+3
    EXPECT_DOUBLE_EQ(out[1], 15.0);  // 4+5+6
}

TEST(CollapseTest, KeepYAverageAndExtremes)
{
    auto avg = collapseVia(makeCollapse("a", 3, 2, 2, CompressionType::AVERAGE, ProjectionAxis::Y), sampleField(), 3, 2);
    EXPECT_DOUBLE_EQ(avg[0], 2.0);
    EXPECT_DOUBLE_EQ(avg[1], 5.0);
    auto mx = collapseVia(makeCollapse("b", 3, 2, 2, CompressionType::MAXIMUM, ProjectionAxis::Y), sampleField(), 3, 2);
    EXPECT_DOUBLE_EQ(mx[0], 3.0);
    EXPECT_DOUBLE_EQ(mx[1], 6.0);
    auto mn = collapseVia(makeCollapse("c", 3, 2, 2, CompressionType::MINIMUM, ProjectionAxis::Y), sampleField(), 3, 2);
    EXPECT_DOUBLE_EQ(mn[0], 1.0);
    EXPECT_DOUBLE_EQ(mn[1], 4.0);
}

// ---------------------------------------------------------------------------
// addInput / single-input
// ---------------------------------------------------------------------------

TEST(CollapseTest, AddInputResizesInputBuffer)
{
    const auto cl = makeCollapse("cl", 4, 4, 8, CompressionType::SUM, ProjectionAxis::X);
    const auto source = makeSource2D("src", 8, 6);
    source->init();
    cl->addInput(source);
    EXPECT_EQ(static_cast<int>(cl->getComponent("input").size()), 8 * 6);
    EXPECT_EQ(cl->getParameters().inputDimensions.size_x, 8);
    EXPECT_EQ(cl->getParameters().inputDimensions.size_y, 6);
}

TEST(CollapseTest, SecondInputIsRejected)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::SUM, ProjectionAxis::X);
    const auto first  = makeSource2D("first", 3, 2);
    const auto second = makeSource2D("second", 5, 4);
    first->init();
    second->init();
    cl->addInput(first);
    cl->addInput(second);
    EXPECT_EQ(cl->getInputs().size(), 1u);
    EXPECT_EQ(static_cast<int>(cl->getComponent("input").size()), 3 * 2);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(CollapseTest, CloneProducesIdenticalOutput)
{
    const auto cl = makeCollapse("cl", 3, 2, 3, CompressionType::SUM, ProjectionAxis::X);
    const auto out = collapseVia(cl, sampleField(), 3, 2);

    const auto clone = std::dynamic_pointer_cast<Collapse>(cl->clone());
    ASSERT_NE(clone, nullptr);
    clone->removeInputs();
    const auto cloneOut = collapseVia(clone, sampleField(), 3, 2);

    ASSERT_EQ(out.size(), cloneOut.size());
    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], cloneOut[i]);
}

TEST(CollapseTest, ToStringReturnsNonEmpty)
{
    const auto cl = makeCollapse("cl", 50, 40, 50, CompressionType::SUM, ProjectionAxis::X);
    EXPECT_FALSE(cl->toString().empty());
}
