#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "elements/normal_noise_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static std::shared_ptr<NormalNoise2D> makeNoise(const std::string& name,
                                                 double amp = 0.2,
                                                 int x_max = 20, int y_max = 20)
{
    ElementCommonParameters cp{ name, ElementDimensions(x_max, y_max, 1.0, 1.0) };
    NormalNoise2DParameters p{ amp };
    return std::make_shared<NormalNoise2D>(cp, p);
}

TEST(NormalNoise2DConstruction, LabelIsNormalNoise2D)
{
    const auto n = makeNoise("nn2d");
    EXPECT_EQ(n->getLabel(), ElementLabel::NORMAL_NOISE_2D);
}

TEST(NormalNoise2DConstruction, SizeIsProductOfDimensions)
{
    const auto n = makeNoise("nn2d", 0.2, 10, 8);
    EXPECT_EQ(n->getSize(), 80);
}

TEST(NormalNoise2DConstruction, UniqueNameMatchesInput)
{
    const auto n = makeNoise("my-noise-2d");
    EXPECT_EQ(n->getUniqueName(), "my-noise-2d");
}

TEST(NormalNoise2DInit, OutputComponentSizeIsCorrect)
{
    const auto n = makeNoise("nn2d", 0.2, 10, 8);
    n->init();
    EXPECT_EQ(static_cast<int>(n->getComponent("output").size()), 80);
}

TEST(NormalNoise2DStep, OutputChangesAcrossSteps)
{
    const auto n = makeNoise("nn2d", 1.0, 10, 10);
    n->init();

    n->step(1.0, 1.0);
    const auto out1 = n->getComponent("output");

    n->step(2.0, 1.0);
    const auto out2 = n->getComponent("output");

    EXPECT_NE(out1, out2);
}

TEST(NormalNoise2DStep, ZeroAmplitudeGivesZeroOutput)
{
    const auto n = makeNoise("nn2d", 0.0, 10, 10);
    n->init();
    n->step(1.0, 1.0);
    for (const double v : n->getComponent("output"))
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(NormalNoise2DParameters, GetParametersRoundtrip)
{
    NormalNoise2DParameters p{ 0.5 };
    ElementCommonParameters cp{ std::string("nn2d"), ElementDimensions(10, 10, 1.0, 1.0) };
    NormalNoise2D n(cp, p);
    EXPECT_EQ(n.getParameters(), p);
}

TEST(NormalNoise2DParameters, SetParametersUpdatesAmplitude)
{
    const auto n = makeNoise("nn2d", 0.2);
    n->init();
    NormalNoise2DParameters newP{ 5.0 };
    n->setParameters(newP);
    EXPECT_DOUBLE_EQ(n->getParameters().amplitude, 5.0);
}

TEST(NormalNoise2DClone, CloneHasSameParameters)
{
    const auto n = makeNoise("nn2d", 0.7);
    n->init();
    const auto cloned = std::dynamic_pointer_cast<NormalNoise2D>(n->clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), n->getParameters());
}

TEST(NormalNoise2DToString, NonEmpty)
{
    const auto n = makeNoise("nn2d");
    EXPECT_FALSE(n->toString().empty());
}
