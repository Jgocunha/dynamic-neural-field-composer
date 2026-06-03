#include <gtest/gtest.h>
#include <memory>

#include "elements/resize_2d.h"
#include "elements/gauss_stimulus_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Resize2D> makeResize2D(const std::string& name,
    const int inX, const int inY, const int outX, const int outY,
    const InterpolationMethod method = InterpolationMethod::LINEAR)
{
    const ElementDimensions inDim{ inX, inY, 1.0, 1.0 };
    const Resize2DParameters rp{ method, inDim };
    const ElementCommonParameters cp{ name, ElementDimensions{ outX, outY, 1.0, 1.0 } };
    return std::make_shared<Resize2D>(cp, rp);
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

// Connect a known y-major field to a Resize2D, step, and return the output.
static std::vector<double> resampleVia(const std::shared_ptr<Resize2D>& resize,
    const std::vector<double>& field, const int sizeX, const int sizeY)
{
    const auto source = makeSource2D("src", sizeX, sizeY);
    source->init();
    resize->addInput(source);
    resize->init();

    auto* out = source->getComponentPtr("output");
    std::copy(field.begin(), field.end(), out->begin());

    resize->step(0.0, 1.0);
    return resize->getComponent("output");
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(Resize2DTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeResize2D("rz", 20, 20, 40, 40));
}

TEST(Resize2DTest, HasCorrectLabel)
{
    const auto rz = makeResize2D("rz", 20, 20, 40, 40);
    EXPECT_EQ(rz->getLabel(), ElementLabel::RESIZE_2D);
}

TEST(Resize2DTest, OutputSizeMatchesDeclaredDimension)
{
    const auto rz = makeResize2D("rz", 20, 20, 40, 30);
    rz->init();
    EXPECT_EQ(static_cast<int>(rz->getComponent("output").size()), 40 * 30);
}

TEST(Resize2DTest, InputSizeMatchesInputDimensions)
{
    const auto rz = makeResize2D("rz", 16, 12, 40, 30);
    rz->init();
    EXPECT_EQ(static_cast<int>(rz->getComponent("input").size()), 16 * 12);
}

TEST(Resize2DTest, AddInputResizesInputBuffer)
{
    const auto rz = makeResize2D("rz", 5, 5, 40, 40);
    const auto source = makeSource2D("src", 8, 6);
    source->init();
    rz->addInput(source);
    EXPECT_EQ(static_cast<int>(rz->getComponent("input").size()), 8 * 6);
    EXPECT_EQ(rz->getParameters().inputDimensions.size_x, 8);
    EXPECT_EQ(rz->getParameters().inputDimensions.size_y, 6);
}

// ---------------------------------------------------------------------------
// Numerical correctness
// ---------------------------------------------------------------------------

TEST(Resize2DTest, IdentityWhenSizesEqual)
{
    const auto rz = makeResize2D("rz", 3, 2, 3, 2, InterpolationMethod::LINEAR);
    // y-major: field[y*3 + x]
    const std::vector<double> field{ 1, 2, 3,
                                     4, 5, 6 };
    const auto out = resampleVia(rz, field, 3, 2);
    ASSERT_EQ(out.size(), field.size());
    for (size_t i = 0; i < field.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], field[i]);
}

TEST(Resize2DTest, LinearUpsampleSeparablePlane)
{
    // Input 2x2 plane f(x,y) = x + 10*y (y-major), upsampled to 3x3.
    // Linear separable interpolation of a bilinear plane reproduces the plane.
    // pos along each axis: i*(N-1)/(M-1) = i*1/2 = 0.5 i
    const auto rz = makeResize2D("rz", 2, 2, 3, 3, InterpolationMethod::LINEAR);
    const std::vector<double> field{ 0.0,  1.0,    // y=0: x=0,1
                                     10.0, 11.0 };  // y=1: x=0,1
    const auto out = resampleVia(rz, field, 2, 2);
    ASSERT_EQ(out.size(), 9u);
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x)
        {
            const double expected = 0.5 * x + 10.0 * (0.5 * y);
            EXPECT_NEAR(out[y * 3 + x], expected, 1e-9)
                << "at x=" << x << " y=" << y;
        }
}

TEST(Resize2DTest, NearestNeighbourSeparable)
{
    // 2x2 -> 3x3 nearest. pos 0,0.5,1 -> round -> 0,1,1 on each axis.
    const auto rz = makeResize2D("rz", 2, 2, 3, 3, InterpolationMethod::NEAREST);
    const std::vector<double> field{ 1.0, 2.0,
                                     3.0, 4.0 };
    const auto out = resampleVia(rz, field, 2, 2);
    ASSERT_EQ(out.size(), 9u);
    // index map per axis: out 0->in 0, out 1->in 1, out 2->in 1
    const int mapAxis[3] = { 0, 1, 1 };
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x)
            EXPECT_DOUBLE_EQ(out[y * 3 + x], field[mapAxis[y] * 2 + mapAxis[x]]);
}

// ---------------------------------------------------------------------------
// Parameter update / clone
// ---------------------------------------------------------------------------

TEST(Resize2DTest, SetParametersChangesMethod)
{
    const auto rz = makeResize2D("rz", 4, 4, 8, 8, InterpolationMethod::LINEAR);
    Resize2DParameters p = rz->getParameters();
    p.method = InterpolationMethod::CUBIC;
    rz->setParameters(p);
    EXPECT_EQ(rz->getParameters().method, InterpolationMethod::CUBIC);
}

TEST(Resize2DTest, CloneProducesIdenticalOutput)
{
    const auto rz = makeResize2D("rz", 2, 2, 3, 3, InterpolationMethod::LINEAR);
    const std::vector<double> field{ 0.0, 1.0, 10.0, 11.0 };
    const auto out = resampleVia(rz, field, 2, 2);

    const auto clone = std::dynamic_pointer_cast<Resize2D>(rz->clone());
    ASSERT_NE(clone, nullptr);
    // The clone copies the original's input connection; clear it so resampleVia
    // wires a single fresh source (otherwise two sources would be summed).
    clone->removeInputs();
    const auto cloneOut = resampleVia(clone, field, 2, 2);

    ASSERT_EQ(out.size(), cloneOut.size());
    for (size_t i = 0; i < out.size(); ++i)
        EXPECT_DOUBLE_EQ(out[i], cloneOut[i]);
}

TEST(Resize2DTest, ToStringReturnsNonEmpty)
{
    const auto rz = makeResize2D("rz", 20, 20, 40, 40);
    EXPECT_FALSE(rz->toString().empty());
}
