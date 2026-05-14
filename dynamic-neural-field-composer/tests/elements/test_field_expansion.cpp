#include <gtest/gtest.h>

#include "elements/field_expansion.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static ElementCommonParameters makeCP(const std::string& name, int xmax = 10, int ymax = 8)
{
    return ElementCommonParameters{ name, ElementDimensions{ xmax, ymax, 1.0, 1.0 } };
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(FieldExpansionConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(FieldExpansion(makeCP("fe"), FieldExpansionParameters{ 0 }));
    EXPECT_NO_THROW(FieldExpansion(makeCP("fe"), FieldExpansionParameters{ 1 }));
}

TEST(FieldExpansionConstruction, LabelIsFieldExpansion)
{
    FieldExpansion fe(makeCP("fe"), FieldExpansionParameters{});
    EXPECT_EQ(fe.getLabel(), ElementLabel::FIELD_EXPANSION);
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(FieldExpansionInit, OutputSizeIsSizeXTimesSizeY)
{
    FieldExpansion fe(makeCP("fe", 10, 8), FieldExpansionParameters{ 0 });
    fe.init();
    EXPECT_EQ(static_cast<int>(fe.getComponent("output").size()), 10 * 8);
}

TEST(FieldExpansionInit, InputSizeIsSizeXWhenAxis0)
{
    FieldExpansion fe(makeCP("fe", 10, 8), FieldExpansionParameters{ 0 });
    fe.init();
    EXPECT_EQ(static_cast<int>(fe.getComponent("input").size()), 10);
}

TEST(FieldExpansionInit, InputSizeIsSizeYWhenAxis1)
{
    FieldExpansion fe(makeCP("fe", 10, 8), FieldExpansionParameters{ 1 });
    fe.init();
    EXPECT_EQ(static_cast<int>(fe.getComponent("input").size()), 8);
}

TEST(FieldExpansionInit, OutputIsZeroAfterInit)
{
    FieldExpansion fe(makeCP("fe"), FieldExpansionParameters{ 0 });
    fe.init();
    for (double v : fe.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// step() — expansion math
// ---------------------------------------------------------------------------

TEST(FieldExpansionStep, ExpandsAlongYFromXInputAxis0)
{
    // input size=2 (size_x), size_y=3
    // input[0]=10, input[1]=20
    // output[xi*3+yi] = input[xi]
    // → row 0: 10,10,10; row 1: 20,20,20
    FieldExpansion fe(makeCP("fe", 2, 3), FieldExpansionParameters{ 0 });
    fe.init();

    auto* inp = fe.getComponentPtr("input");
    ASSERT_NE(inp, nullptr);
    (*inp)[0] = 10.0;
    (*inp)[1] = 20.0;

    fe.step(0.0, 1.0);

    const auto out = fe.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0 * 3 + 0], 10.0);
    EXPECT_DOUBLE_EQ(out[0 * 3 + 1], 10.0);
    EXPECT_DOUBLE_EQ(out[0 * 3 + 2], 10.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 0], 20.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 1], 20.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 2], 20.0);
}

TEST(FieldExpansionStep, ExpandsAlongXFromYInputAxis1)
{
    // input size=3 (size_y), size_x=2
    // input[0]=1, input[1]=2, input[2]=3
    // output[xi*3+yi] = input[yi]
    // → col 0: 1,1; col 1: 2,2; col 2: 3,3
    FieldExpansion fe(makeCP("fe", 2, 3), FieldExpansionParameters{ 1 });
    fe.init();

    auto* inp = fe.getComponentPtr("input");
    (*inp)[0] = 1.0;
    (*inp)[1] = 2.0;
    (*inp)[2] = 3.0;

    fe.step(0.0, 1.0);

    const auto out = fe.getComponent("output");
    EXPECT_DOUBLE_EQ(out[0 * 3 + 0], 1.0);
    EXPECT_DOUBLE_EQ(out[0 * 3 + 1], 2.0);
    EXPECT_DOUBLE_EQ(out[0 * 3 + 2], 3.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 0], 1.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 1], 2.0);
    EXPECT_DOUBLE_EQ(out[1 * 3 + 2], 3.0);
}

TEST(FieldExpansionStep, ZeroInputGivesZeroOutput)
{
    FieldExpansion fe(makeCP("fe"), FieldExpansionParameters{ 0 });
    fe.init();
    fe.step(0.0, 1.0);
    for (double v : fe.getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// setParameters / getParameters
// ---------------------------------------------------------------------------

TEST(FieldExpansionParameters, GetParametersRoundtrip)
{
    const FieldExpansionParameters p{ 1 };
    FieldExpansion fe(makeCP("fe"), p);
    EXPECT_EQ(fe.getParameters(), p);
}

TEST(FieldExpansionParameters, SetParametersChangesInputSize)
{
    FieldExpansion fe(makeCP("fe", 10, 8), FieldExpansionParameters{ 0 });
    fe.init();
    EXPECT_EQ(static_cast<int>(fe.getComponent("input").size()), 10);

    fe.setParameters(FieldExpansionParameters{ 1 });
    EXPECT_EQ(static_cast<int>(fe.getComponent("input").size()), 8);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(FieldExpansionClone, CloneProducesSameOutput)
{
    FieldExpansion fe(makeCP("fe", 2, 3), FieldExpansionParameters{ 0 });
    fe.init();
    auto* inp = fe.getComponentPtr("input");
    (*inp)[0] = 5.0; (*inp)[1] = 7.0;
    fe.step(0.0, 1.0);

    const auto cloned = std::dynamic_pointer_cast<FieldExpansion>(fe.clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(fe.getComponent("output"), cloned->getComponent("output"));
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(FieldExpansionToString, NonEmpty)
{
    FieldExpansion fe(makeCP("fe"), FieldExpansionParameters{});
    EXPECT_FALSE(fe.toString().empty());
}
