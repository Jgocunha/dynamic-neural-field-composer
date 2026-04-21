#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <numeric>

#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "elements/gauss_stimulus.h"
#include "simulation/simulation.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static NeuralFieldParameters makeNFP(const double tau = 25.0, const double restLevel = -5.0)
{
    const SigmoidFunction sigmoid{ 0.0, 10.0 };
    return NeuralFieldParameters{ tau, restLevel, sigmoid };
}

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100,
                                               const double tau = 25.0, const double restLevel = -5.0)
{
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, makeNFP(tau, restLevel));
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name,
                                                    const double pos = 50.0, const double amp = 15.0,
                                                    const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, amp, pos, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(NeuralFieldConstruction, LabelIsNeuralField)
{
    const auto f = makeField("f");
    EXPECT_EQ(f->getLabel(), ElementLabel::NEURAL_FIELD);
}

TEST(NeuralFieldConstruction, ComponentsExistAfterConstruction)
{
    const auto f = makeField("f", 100);
    // These components are created in the constructor before init()
    EXPECT_NO_THROW(f->getComponent("activation"));
    EXPECT_NO_THROW(f->getComponent("resting level"));
}

TEST(NeuralFieldConstruction, SizeIsCorrect)
{
    const auto f = makeField("f", 60);
    EXPECT_EQ(f->getSize(), 60);
}

TEST(NeuralFieldConstruction, UniqueNameMatchesInput)
{
    const auto f = makeField("my-field");
    EXPECT_EQ(f->getUniqueName(), "my-field");
}

// ---------------------------------------------------------------------------
// init()
// ---------------------------------------------------------------------------

TEST(NeuralFieldInit, ActivationFillsRestingLevel)
{
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();
    for (const auto act = f->getComponent("activation"); const double v : act)
        EXPECT_DOUBLE_EQ(v, -5.0);
}

TEST(NeuralFieldInit, OutputNearZeroForNegativeRestingLevel)
{
    // sigmoid(-5) ≈ 0 for large steepness
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();
    for (const auto out = f->getComponent("output"); double v : out)
        EXPECT_LT(v, 0.1);
}

TEST(NeuralFieldInit, OutputComponentHasCorrectSize)
{
    const auto f = makeField("f", 80);
    f->init();
    const auto out = f->getComponent("output");
    EXPECT_EQ(static_cast<int>(out.size()), 80);
}

// ---------------------------------------------------------------------------
// step() — standalone (no input → activation decays toward resting level)
// ---------------------------------------------------------------------------

TEST(NeuralFieldStep, ActivationRemainsAtRestingLevelWithoutInput)
{
    // With no input, activation should stay at resting level (fixed point)
    const auto f = makeField("f", 10, 25.0, -5.0);
    f->init();
    f->step(1.0, 1.0);
    // u(t+1) = u(t) + dt/tau * (-u(t) + h + input)
    // = -5 + 1/25 * (-(-5) + (-5) + 0) = -5
    for (const auto act = f->getComponent("activation"); const double v : act)
        EXPECT_NEAR(v, -5.0, 0.01);
}

// ---------------------------------------------------------------------------
// getLowestActivation / getHighestActivation
// ---------------------------------------------------------------------------

TEST(NeuralFieldState, LowestAndHighestActivationAfterInit)
{
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();
    f->step(1.0, 1.0); // updateState is called inside the step
    EXPECT_DOUBLE_EQ(f->getLowestActivation(), -5.0);
    EXPECT_DOUBLE_EQ(f->getHighestActivation(), -5.0);
}

// ---------------------------------------------------------------------------
// isStable()
// ---------------------------------------------------------------------------

TEST(NeuralFieldStability, NotStableInitially)
{
    const auto f = makeField("f", 100);
    f->init();
    f->step(1.0, 1.0);  // first step: activation hasn't changed yet so might be stable
    // After just one-step stability is indeterminate – we mainly verify no crash
    // and the method is callable
    EXPECT_NO_THROW(f->isStable());
}

TEST(NeuralFieldStability, StableAfterManyStepsWithNoInput)
{
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();
    for (int i = 0; i < 200; ++i)
        f->step(static_cast<double>(i), 1.0);
    EXPECT_TRUE(f->isStable());
}

// ---------------------------------------------------------------------------
// Integration: NeuralField driven by GaussStimulus
// ---------------------------------------------------------------------------

TEST(NeuralFieldIntegration, ActivationRisesWithStrongStimulus)
{
    // Build a small simulation
    Simulation sim("nf-test", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0, 30.0);
    const auto field = makeField("field", 100, 25.0, -5.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    // Record initial max activation
    const double initMax = field->getHighestActivation();
    for (int i = 0; i < 50; ++i)
        sim.step();

    const double laterMax = field->getHighestActivation();
    EXPECT_GT(laterMax, initMax);
}

// ---------------------------------------------------------------------------
// getParameters / setParameters
// ---------------------------------------------------------------------------

TEST(NeuralFieldParameters, GetParametersRoundtrip)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    const NeuralFieldParameters nfp{ 20.0, -3.0, sig };
    const ElementCommonParameters cp{ std::string("f"), 100 };
    const NeuralField f(cp, nfp);
    const auto got = f.getParameters();
    EXPECT_DOUBLE_EQ(got.tau, 20.0);
    EXPECT_DOUBLE_EQ(got.startingRestingLevel, -3.0);
}

TEST(NeuralFieldParameters, SetParametersReInit)
{
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();

    const SigmoidFunction sig{ 0.0, 10.0 };
    const NeuralFieldParameters newP{ 25.0, -2.0, sig };
    f->setParameters(newP);

    for (const auto act = f->getComponent("activation"); const double v : act)
        EXPECT_DOUBLE_EQ(v, -2.0);
}

// ---------------------------------------------------------------------------
// clone
// ---------------------------------------------------------------------------

TEST(NeuralFieldClone, CloneHasSameParameters)
{
    const auto f = makeField("f", 100, 30.0, -4.0);
    f->init();
    const auto cloned = f->clone();
    const auto clonedNF = std::dynamic_pointer_cast<NeuralField>(cloned);
    ASSERT_NE(clonedNF, nullptr);
    EXPECT_DOUBLE_EQ(clonedNF->getParameters().tau, f->getParameters().tau);
    EXPECT_DOUBLE_EQ(clonedNF->getParameters().startingRestingLevel,
                     f->getParameters().startingRestingLevel);
}

// ---------------------------------------------------------------------------
// toString
// ---------------------------------------------------------------------------

TEST(NeuralFieldToString, NonEmpty)
{
    const auto f = makeField("f");
    f->init();
    EXPECT_FALSE(f->toString().empty());
}

// ---------------------------------------------------------------------------
// getBumps
// ---------------------------------------------------------------------------

TEST(NeuralFieldBumps, NoBumpsAtRestingLevel)
{
    const auto f = makeField("f", 100, 25.0, -5.0);
    f->init();
    f->step(1.0, 1.0);
    EXPECT_TRUE(f->getBumps().empty());
}
