#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "elements/normal_noise.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NormalNoise> makeNoise(const std::string& name,
                                               const double amp = 0.2, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    NormalNoiseParameters nnp{ amp };
    return std::make_shared<NormalNoise>(cp, nnp);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(NormalNoiseConstruction, LabelIsNormalNoise)
{
    const auto n = makeNoise("noise");
    EXPECT_EQ(n->getLabel(), ElementLabel::NORMAL_NOISE);
}

TEST(NormalNoiseConstruction, SizeIsCorrect)
{
    const auto n = makeNoise("noise", 0.2, 60);
    EXPECT_EQ(n->getSize(), 60);
}

TEST(NormalNoiseConstruction, UniqueNameMatchesInput)
{
    const auto n = makeNoise("my-noise");
    EXPECT_EQ(n->getUniqueName(), "my-noise");
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(NormalNoiseInit, OutputComponentSizeIsCorrect)
{
    const auto n = makeNoise("noise", 0.2, 80);
    n->init();
    const auto out = n->getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 80);
}

// ---------------------------------------------------------------------------
// step() — noise should produce varying output each call
// ---------------------------------------------------------------------------

TEST(NormalNoiseStep, OutputChangesAcrossSteps)
{
    const auto n = makeNoise("noise", 1.0, 100);
    n->init();

    n->step(1.0, 1.0);
    const auto out1 = n->getComponent("output");

    n->step(2.0, 1.0);
    const auto out2 = n->getComponent("output");

    // Random noise → outputs are almost certainly different
    EXPECT_NE(out1, out2);
}

TEST(NormalNoiseStep, OutputValuesAreScaledByAmplitude)
{
    // With amplitude=0, all output should be zero
    const auto n = makeNoise("noise", 0.0, 50);
    n->init();
    n->step(1.0, 1.0);
    for (const auto out = n->getComponent("output"); const double v : out)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

TEST(NormalNoiseStep, LargeAmplitudeProducesLargerValues)
{
    const auto nSmall = makeNoise("n1", 0.01, 1000);
    const auto nLarge = makeNoise("n2", 100.0, 1000);

    nSmall->init();
    nLarge->init();

    nSmall->step(1.0, 1.0);
    nLarge->step(1.0, 1.0);

    const auto outSmall = nSmall->getComponent("output");
    const auto outLarge = nLarge->getComponent("output");

    double rmsSmall = 0.0, rmsLarge = 0.0;
    for (int i = 0; i < 1000; ++i)
    {
        rmsSmall += outSmall[i] * outSmall[i];
        rmsLarge += outLarge[i] * outLarge[i];
    }
    EXPECT_GT(rmsLarge, rmsSmall);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(NormalNoiseParameters, GetParametersRoundtrip)
{
    const NormalNoiseParameters nnp{ 0.5 };
    const ElementCommonParameters cp{ std::string("noise"), 100 };
    const NormalNoise n(cp, nnp);
    EXPECT_EQ(n.getParameters(), nnp);
}

TEST(NormalNoiseParameters, EqualityOperatorSame)
{
    const NormalNoiseParameters a{ 0.3 };
    const NormalNoiseParameters b{ 0.3 };
    EXPECT_TRUE(a == b);
}

TEST(NormalNoiseParameters, EqualityOperatorDifferent)
{
    const NormalNoiseParameters a{ 0.3 };
    const NormalNoiseParameters b{ 0.4 };
    EXPECT_FALSE(a == b);
}

TEST(NormalNoiseParameters, SetParametersUpdatesAmplitude)
{
    const auto n = makeNoise("noise", 0.2, 50);
    n->init();
    const NormalNoiseParameters newP{ 5.0 };
    n->setParameters(newP);
    EXPECT_DOUBLE_EQ(n->getParameters().amplitude, 5.0);
}

TEST(NormalNoiseParameters, ToStringIsNonEmpty)
{
    const NormalNoiseParameters nnp{ 0.5 };
    EXPECT_FALSE(nnp.toString().empty());
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(NormalNoiseClone, CloneHasSameParameters)
{
    const auto n = makeNoise("noise", 0.7, 80);
    n->init();
    const auto cloned = n->clone();
    auto* cn = dynamic_cast<NormalNoise*>(cloned.get());
    ASSERT_NE(cn, nullptr);
    EXPECT_EQ(cn->getParameters(), n->getParameters());
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(NormalNoiseToString, NonEmpty)
{
    const auto n = makeNoise("noise");
    EXPECT_FALSE(n->toString().empty());
}
