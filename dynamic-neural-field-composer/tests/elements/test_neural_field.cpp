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

// ---------------------------------------------------------------------------
// AbsSigmoidFunction as activation function
// ---------------------------------------------------------------------------

TEST(NeuralFieldAbsSigmoid, ConstructionAndStepDoNotThrow)
{
    const AbsSigmoidFunction sig{ 0.0, 100.0 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    const ElementCommonParameters cp{ std::string("f"), 100 };
    const auto f = std::make_shared<NeuralField>(cp, nfp);
    EXPECT_NO_THROW(f->init());
    EXPECT_NO_THROW(f->step(0.0, 1.0));
}

TEST(NeuralFieldAbsSigmoid, OutputNearZeroAtRestingLevel)
{
    // AbsSigmoid(0, beta=100): at x=-5 output ≈ 0 (deeply sub-threshold)
    const AbsSigmoidFunction sig{ 0.0, 100.0 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    const ElementCommonParameters cp{ std::string("f"), 100 };
    const auto f = std::make_shared<NeuralField>(cp, nfp);
    f->init();
    f->step(0.0, 1.0);
    for (const auto out = f->getComponent("output"); const double v : out)
        EXPECT_LT(v, 0.1);
}

TEST(NeuralFieldAbsSigmoid, RisesWithStrongStimulus)
{
    Simulation sim("abs-sigmoid-test", 1.0, 0.0, 0.0);

    const AbsSigmoidFunction sig{ 0.0, 100.0 };
    const NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    const ElementCommonParameters cp{ std::string("field"), 100 };
    const auto field = std::make_shared<NeuralField>(cp, nfp);

    const auto stim = makeStimulus("stim", 50.0, 30.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    const double initMax = field->getHighestActivation();
    for (int i = 0; i < 50; ++i)
        sim.step();

    EXPECT_GT(field->getHighestActivation(), initMax);
}

TEST(NeuralFieldAbsSigmoid, GetSetParametersRoundtrip)
{
    const AbsSigmoidFunction sig{ 0.5, 50.0 };
    const NeuralFieldParameters nfp{ 20.0, -3.0, sig };
    const ElementCommonParameters cp{ std::string("f"), 50 };
    NeuralField f(cp, nfp);
    const auto got = f.getParameters();
    EXPECT_DOUBLE_EQ(got.tau, 20.0);
    EXPECT_DOUBLE_EQ(got.startingRestingLevel, -3.0);
    // Verify the stored function is AbsSigmoidFunction
    const auto* stored = dynamic_cast<const AbsSigmoidFunction*>(got.activationFunction.get());
    ASSERT_NE(stored, nullptr);
    EXPECT_NEAR(stored->getXShift(), 0.5, 1e-9);
    EXPECT_NEAR(stored->getBeta(),   50.0, 1e-9);
}

// ---------------------------------------------------------------------------
// getBumps — positive detection (rewritten updateBumps path)
// ---------------------------------------------------------------------------

TEST(NeuralFieldBumps, BumpDetectedWhenActivationAboveThreshold)
{
    // Run a field with a strong enough stimulus to push activation above the bump threshold.
    Simulation sim("bump-detect", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0, 30.0);
    const auto field = makeField("field", 100, 25.0, -5.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    for (int i = 0; i < 200; ++i)
        sim.step();

    const auto bumps = field->getBumps();
    EXPECT_FALSE(bumps.empty());
    EXPECT_GE(bumps.front().amplitude, 0.0);
    EXPECT_GT(bumps.front().width, 0.0);
}

TEST(NeuralFieldBumps, BumpCentroidNearStimulusPosition)
{
    Simulation sim("bump-centroid", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0, 30.0);
    const auto field = makeField("field", 100, 25.0, -5.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    for (int i = 0; i < 200; ++i)
        sim.step();

    const auto bumps = field->getBumps();
    ASSERT_FALSE(bumps.empty());
    // Centroid should be close to the stimulus position (50)
    EXPECT_NEAR(bumps.front().centroid, 50.0, 10.0);
}

// ---------------------------------------------------------------------------
// getLowestActivation / getHighestActivation — with real activation gradient
// ---------------------------------------------------------------------------

TEST(NeuralFieldState, HighestActivationAboveRestingLevelUnderStimulus)
{
    Simulation sim("hi-lo-test", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0, 30.0);
    const auto field = makeField("field", 100, 25.0, -5.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    for (int i = 0; i < 100; ++i)
        sim.step();

    EXPECT_GT(field->getHighestActivation(), field->getLowestActivation());
    EXPECT_GT(field->getHighestActivation(), -5.0);
}

// ---------------------------------------------------------------------------
// Bump velocity — non-zero when bump is driven to move
// ---------------------------------------------------------------------------

TEST(NeuralFieldBumps, BumpVelocityNonZeroWhenStimulusMoves)
{
    // Settle at position 50, then shift stimulus 10 positions to the right.
    // A 10-position shift keeps the bump intact (no split) while the centroid
    // tracks the new stimulus position over ~100 time constants.
    // We accumulate the peak velocity seen over 200 steps: if the velocity
    // computation in updateBumps() works, at least one step must show non-zero.
    Simulation sim("velocity-test", 1.0, 0.0, 0.0);
    const auto stim  = makeStimulus("stim", 50.0, 30.0);
    const auto field = makeField("field", 100, 25.0, -5.0);
    sim.addElement(stim);
    sim.addElement(field);
    sim.createInteraction("stim", "output", "field");
    sim.init();

    for (int i = 0; i < 200; ++i)
        sim.step();

    ASSERT_FALSE(field->getBumps().empty());

    const auto stimCast = std::dynamic_pointer_cast<GaussStimulus>(sim.getElement("stim"));
    ASSERT_NE(stimCast, nullptr);
    GaussStimulusParameters gsp{ 5.0, 30.0, 60.0, true, false };
    stimCast->setParameters(gsp);

    double maxVelocity = 0.0;
    for (int i = 0; i < 200; ++i)
    {
        sim.step();
        for (const auto& b : field->getBumps())
            maxVelocity = std::max(maxVelocity, std::abs(b.velocity));
    }

    EXPECT_GT(maxVelocity, 1e-9);
}
