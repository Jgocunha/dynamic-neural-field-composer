#include <gtest/gtest.h>
#include <numeric>

#include "elements/activation_function.h"
#include "elements/field_projection.h"
#include "elements/neural_field.h"
#include "elements/neural_field_2d.h"
#include "simulation/simulation.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static std::shared_ptr<NeuralField> makeField1D(const std::string& name, int size = 10)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    const NeuralFieldParameters parameters{ 25.0, -5.0, sig };
    return std::make_shared<NeuralField>(ElementCommonParameters{ name, size }, parameters);
}

static std::shared_ptr<NeuralField2D> makeField2D(const std::string& name, int sizeX = 10, int sizeY = 8)
{
    const NeuralField2DParameters parameters{};
    return std::make_shared<NeuralField2D>(ElementCommonParameters{ name, ElementDimensions{ sizeX, sizeY, 1.0, 1.0 } }, parameters);
}

static ElementCommonParameters makeCP(const std::string& name, int xmax = 10, int ymax = 8)
{
    return ElementCommonParameters{ name, ElementDimensions{ xmax, ymax, 1.0, 1.0 } };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(FieldProjectionConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(FieldProjection(makeCP("fp"), FieldProjectionParameters{ 0 }));
    EXPECT_NO_THROW(FieldProjection(makeCP("fp"), FieldProjectionParameters{ 1 }));
}

TEST(FieldProjectionConstruction, LabelIsFieldProjection)
{
    FieldProjection fp(makeCP("fp"), FieldProjectionParameters{});
    EXPECT_EQ(fp.getLabel(), ElementLabel::FIELD_PROJECTION);
}

TEST(FieldProjectionConstruction, OutputSizeMatchesProjectedAxisBeforeInit)
{
    FieldProjection fpX(makeCP("fpX", 10, 8), FieldProjectionParameters{ 0 });
    EXPECT_EQ(static_cast<int>(fpX.getComponent("output").size()), 10);

    FieldProjection fpY(makeCP("fpY", 10, 8), FieldProjectionParameters{ 1 });
    EXPECT_EQ(static_cast<int>(fpY.getComponent("output").size()), 8);
}

TEST(FieldProjectionIntegration, ConnectsTwoDFieldToOneDField)
{
    Simulation sim("field-projection-connection", 1.0, 0.0, 0.0);

    const auto source = makeField2D("source", 10, 8);
    const auto projection = std::make_shared<FieldProjection>(
        ElementCommonParameters{ "projection", ElementDimensions{ 10, 8, 1.0, 1.0 } },
        FieldProjectionParameters{ 0 });
    const auto target = makeField1D("target", 10);

    sim.addElement(source);
    sim.addElement(projection);
    sim.addElement(target);

    EXPECT_NO_THROW(sim.createInteraction("source", "output", "projection"));
    EXPECT_NO_THROW(sim.createInteraction("projection", "output", "target"));

    EXPECT_TRUE(projection->hasInput("source", "output"));
    EXPECT_TRUE(target->hasInput("projection", "output"));
    EXPECT_EQ(static_cast<int>(projection->getComponent("input").size()), 80);
    EXPECT_EQ(static_cast<int>(projection->getComponent("output").size()), 10);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(FieldProjectionInit, OutputSizeAlongXWhenAxis0)
{
    FieldProjection fp(makeCP("fp", 10, 8), FieldProjectionParameters{ 0 });
    fp.init();
    EXPECT_EQ(static_cast<int>(fp.getComponent("output").size()), 10);
}

TEST(FieldProjectionInit, OutputSizeAlongYWhenAxis1)
{
    FieldProjection fp(makeCP("fp", 10, 8), FieldProjectionParameters{ 1 });
    fp.init();
    EXPECT_EQ(static_cast<int>(fp.getComponent("output").size()), 8);
}

TEST(FieldProjectionInit, OutputIsZeroAfterInit)
{
    FieldProjection fp(makeCP("fp"), FieldProjectionParameters{ 0 });
    fp.init();
    for (double v : fp.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// step() — projection math
// ---------------------------------------------------------------------------

TEST(FieldProjectionStep, SumsOverYAxis0)
{
    // 2x3 field with input[xi*3+yi] = xi+1  (row 0 = 1,1,1; row 1 = 2,2,2)
    // projectionAxis=0 → output[xi] = sum over Y → output[0]=3, output[1]=6
    FieldProjection fp(makeCP("fp", 2, 3), FieldProjectionParameters{ 0 });
    fp.init();

    // Directly populate input component to simulate updateInput
    auto* inp = fp.getComponentPtr("input");
    ASSERT_NE(inp, nullptr);
    ASSERT_EQ(static_cast<int>(inp->size()), 2 * 3);
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 3; ++yi)
            (*inp)[xi * 3 + yi] = static_cast<double>(xi + 1);

    fp.step(0.0, 1.0);

    const auto out = fp.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0], 3.0);
    EXPECT_DOUBLE_EQ(out[1], 6.0);
}

TEST(FieldProjectionStep, SumsOverXAxis1)
{
    // 2x3 field with input[xi*3+yi] = yi+1  (col 0 = 1,1; col 1 = 2,2; col 2 = 3,3)
    // projectionAxis=1 → output[yi] = sum over X → output[0]=2, output[1]=4, output[2]=6
    FieldProjection fp(makeCP("fp", 2, 3), FieldProjectionParameters{ 1 });
    fp.init();

    auto* inp = fp.getComponentPtr("input");
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 3; ++yi)
            (*inp)[xi * 3 + yi] = static_cast<double>(yi + 1);

    fp.step(0.0, 1.0);

    const auto out = fp.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0], 2.0);
    EXPECT_DOUBLE_EQ(out[1], 4.0);
    EXPECT_DOUBLE_EQ(out[2], 6.0);
}

TEST(FieldProjectionStep, ZeroInputGivesZeroOutput)
{
    FieldProjection fp(makeCP("fp"), FieldProjectionParameters{ 0 });
    fp.init();
    fp.step(0.0, 1.0);
    for (double v : fp.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// setParameters / getParameters
// ---------------------------------------------------------------------------

TEST(FieldProjectionParameters, GetParametersRoundtrip)
{
    const FieldProjectionParameters p{ 1 };
    FieldProjection fp(makeCP("fp"), p);
    EXPECT_EQ(fp.getParameters(), p);
}

TEST(FieldProjectionParameters, SetParametersChangesOutputSize)
{
    FieldProjection fp(makeCP("fp", 10, 8), FieldProjectionParameters{ 0 });
    fp.init();
    EXPECT_EQ(static_cast<int>(fp.getComponent("output").size()), 10);

    fp.setParameters(FieldProjectionParameters{ 1 });
    EXPECT_EQ(static_cast<int>(fp.getComponent("output").size()), 8);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(FieldProjectionClone, CloneProducesSameOutput)
{
    FieldProjection fp(makeCP("fp", 2, 3), FieldProjectionParameters{ 0 });
    fp.init();
    auto* inp = fp.getComponentPtr("input");
    for (int i = 0; i < 6; ++i) (*inp)[i] = static_cast<double>(i);
    fp.step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<FieldProjection>(fp.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(fp.getComponent("output"), cloned->getComponent("output"));
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(FieldProjectionToString, NonEmpty)
{
    FieldProjection fp(makeCP("fp"), FieldProjectionParameters{});
    EXPECT_FALSE(fp.toString().empty());
}

// ---------------------------------------------------------------------------
// Compression types — Average, Maximum, Minimum
// ---------------------------------------------------------------------------

TEST(FieldProjectionStep, AverageAxis0)
{
    // 2x3 field: row 0 = [1,1,1], row 1 = [2,2,2]
    // axis 0, AVERAGE → output[0] = 3/3 = 1, output[1] = 6/3 = 2
    FieldProjection fp(makeCP("fp", 2, 3),
        FieldProjectionParameters{ 0, FieldProjectionCompression::AVERAGE });
    fp.init();

    auto* inp = fp.getComponentPtr("input");
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 3; ++yi)
            (*inp)[xi * 3 + yi] = static_cast<double>(xi + 1);

    fp.step(0.0, 1.0);

    const auto out = fp.getComponent("output");
    EXPECT_NEAR(out[0], 1.0, 1e-9);
    EXPECT_NEAR(out[1], 2.0, 1e-9);
}

TEST(FieldProjectionStep, MaximumAxis1)
{
    // 2x3 field: col 0 = [10, 20], col 1 = [30, 40], col 2 = [50, 60]
    // axis 1, MAXIMUM → output[yi] = max over X → output[0]=20, output[1]=40, output[2]=60
    FieldProjection fp(makeCP("fp", 2, 3),
        FieldProjectionParameters{ 1, FieldProjectionCompression::MAXIMUM });
    fp.init();

    auto* inp = fp.getComponentPtr("input");
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 3; ++yi)
            (*inp)[xi * 3 + yi] = static_cast<double>((xi + 1) * 10 * (yi + 1));

    fp.step(0.0, 1.0);

    const auto out = fp.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0], 20.0);
    EXPECT_DOUBLE_EQ(out[1], 40.0);
    EXPECT_DOUBLE_EQ(out[2], 60.0);
}

TEST(FieldProjectionStep, MinimumAxis0)
{
    // 2x3 field: row 0 = [1,2,3], row 1 = [4,5,6]
    // axis 0, MINIMUM → output[xi] = min over Y → output[0]=1, output[1]=4
    FieldProjection fp(makeCP("fp", 2, 3),
        FieldProjectionParameters{ 0, FieldProjectionCompression::MINIMUM });
    fp.init();

    auto* inp = fp.getComponentPtr("input");
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 3; ++yi)
            (*inp)[xi * 3 + yi] = static_cast<double>(xi * 3 + yi + 1);

    fp.step(0.0, 1.0);

    const auto out = fp.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0], 1.0);
    EXPECT_DOUBLE_EQ(out[1], 4.0);
}

TEST(FieldProjectionParameters, CompressionTypeRoundtrip)
{
    FieldProjection fp(makeCP("fp"),
        FieldProjectionParameters{ 1, FieldProjectionCompression::MAXIMUM });
    EXPECT_EQ(fp.getParameters().compressionType, FieldProjectionCompression::MAXIMUM);

    fp.setParameters(FieldProjectionParameters{ 0, FieldProjectionCompression::MINIMUM });
    EXPECT_EQ(fp.getParameters().compressionType, FieldProjectionCompression::MINIMUM);
    EXPECT_EQ(fp.getParameters().projectionAxis, 0);
}
