#include <gtest/gtest.h>
#include <memory>

#include "elements/stimulus_sum.h"
#include "elements/boost_stimulus.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ElementCommonParameters makeCP(const std::string& name, const int size = 10)
{
    return ElementCommonParameters{ name, size };
}

static std::shared_ptr<BoostStimulus> makeConstantSource(const std::string& name,
                                                          const double amplitude,
                                                          const int size = 10)
{
    auto bs = std::make_shared<BoostStimulus>(makeCP(name, size), BoostStimulusParameters{ amplitude, true });
    bs->init();
    bs->step(0.0, 1.0);
    return bs;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(StimulusSumConstruction, ValidParametersDoNotThrow)
{
    EXPECT_NO_THROW(StimulusSum(makeCP("ss"), StimulusSumParameters()));
}

TEST(StimulusSumConstruction, LabelIsStimulusSum)
{
    StimulusSum ss(makeCP("ss"), StimulusSumParameters());
    EXPECT_EQ(ss.getLabel(), ElementLabel::STIMULUS_SUM);
}

TEST(StimulusSumToString, NonEmpty)
{
    const StimulusSum ss(makeCP("ss"), StimulusSumParameters());
    EXPECT_FALSE(ss.toString().empty());
}

// ---------------------------------------------------------------------------
// step() — summing inputs
// ---------------------------------------------------------------------------

TEST(StimulusSumStep, SumsTwoSameSizeInputs)
{
    auto a = makeConstantSource("a", 1.5, 10);
    auto b = makeConstantSource("b", 2.5, 10);

    auto ss = std::make_shared<StimulusSum>(makeCP("ss", 10), StimulusSumParameters());
    ss->addInput(a);
    ss->addInput(b);
    ss->init();
    ss->step(0.0, 1.0);

    const auto out = ss->getComponent("output");
    for (const double v : out)
        EXPECT_NEAR(v, 4.0, 1e-9);
}

TEST(StimulusSumStep, SumsThreeInputsArbitraryN)
{
    auto a = makeConstantSource("a", 1.0, 8);
    auto b = makeConstantSource("b", 2.0, 8);
    auto c = makeConstantSource("c", 3.0, 8);

    auto ss = std::make_shared<StimulusSum>(makeCP("ss", 8), StimulusSumParameters());
    ss->addInput(a);
    ss->addInput(b);
    ss->addInput(c);
    ss->init();
    ss->step(0.0, 1.0);

    const auto out = ss->getComponent("output");
    for (const double v : out)
        EXPECT_NEAR(v, 6.0, 1e-9);
}

TEST(StimulusSumStep, ZeroInputsRunsWithoutCrashingAndLogsWarning)
{
    auto ss = std::make_shared<StimulusSum>(makeCP("ss", 5), StimulusSumParameters());
    ss->init();

    ::testing::internal::CaptureStdout();
    EXPECT_NO_THROW(ss->step(0.0, 1.0));
    const std::string out = ::testing::internal::GetCapturedStdout();

    EXPECT_NE(out.find("WARNING"), std::string::npos);

    const auto result = ss->getComponent("output");
    for (const double v : result)
        EXPECT_DOUBLE_EQ(v, 0.0);
}

// ---------------------------------------------------------------------------
// addInput — mismatched sizes rejected (inherited from Element::addInput)
// ---------------------------------------------------------------------------

TEST(StimulusSumAddInput, MismatchedSizeInputIsRejected)
{
    auto a = makeConstantSource("a", 1.0, 10);
    auto wrongSize = makeConstantSource("wrong", 1.0, 20);

    auto ss = std::make_shared<StimulusSum>(makeCP("ss", 10), StimulusSumParameters());
    ss->addInput(a);
    ss->addInput(wrongSize);

    EXPECT_EQ(ss->getInputs().size(), 1u);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(StimulusSumClone, CloneIsIndependentAndEquivalent)
{
    StimulusSum ss(makeCP("ss", 6), StimulusSumParameters());
    ss.init();

    const auto cloned = ss.clone();
    const auto clonedSS = std::dynamic_pointer_cast<StimulusSum>(cloned);
    ASSERT_NE(clonedSS, nullptr);
    EXPECT_EQ(clonedSS->getUniqueName(), ss.getUniqueName());
    EXPECT_EQ(clonedSS->getLabel(), ss.getLabel());
    EXPECT_EQ(clonedSS->getSize(), ss.getSize());

    // Independent: mutating the clone's output must not affect the original.
    auto* clonedOut = clonedSS->getComponentPtr("output");
    std::fill(clonedOut->begin(), clonedOut->end(), 42.0);
    const auto originalOut = ss.getComponent("output");
    for (const double v : originalOut)
        EXPECT_DOUBLE_EQ(v, 0.0);
}
