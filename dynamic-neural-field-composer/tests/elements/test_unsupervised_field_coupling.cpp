#include <gtest/gtest.h>
#include <memory>

#include "elements/unsupervised_field_coupling.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "simulation/simulation.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static std::shared_ptr<NeuralField> makeField(const std::string& name, int size = 50)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, ElementDimensions(size, 1.0) };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<UnsupervisedFieldCoupling> makeUFC(const std::string& name,
    int inSize = 50, int outSize = 50, LearningRule rule = LearningRule::HEBB)
{
    ElementDimensions inDim{ inSize, 1.0 };
    UnsupervisedFieldCouplingParameters fcp{ inDim, rule, 1.0, 0.01 };
    ElementCommonParameters cp{ name, ElementDimensions(outSize, 1.0) };
    return std::make_shared<UnsupervisedFieldCoupling>(cp, fcp);
}

// Construction
TEST(UnsupervisedFieldCouplingTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeUFC("ufc"));
}

TEST(UnsupervisedFieldCouplingTest, HasCorrectLabel)
{
    const auto ufc = makeUFC("ufc");
    EXPECT_EQ(ufc->getLabel(), ElementLabel::UNSUPERVISED_FIELD_COUPLING);
}

TEST(UnsupervisedFieldCouplingTest, HasRequiredComponents)
{
    const auto ufc = makeUFC("ufc");
    EXPECT_NO_THROW(ufc->getComponent("input"));
    EXPECT_NO_THROW(ufc->getComponent("output"));
    EXPECT_NO_THROW(ufc->getComponent("weights"));
}

// Initialisation
TEST(UnsupervisedFieldCouplingTest, ComponentSizesAfterInit)
{
    const int inSz = 30, outSz = 50;
    auto ufc = makeUFC("ufc", inSz, outSz);
    ufc->init();
    EXPECT_EQ(static_cast<int>(ufc->getComponent("input").size()),   inSz);
    EXPECT_EQ(static_cast<int>(ufc->getComponent("output").size()),  outSz);
    EXPECT_EQ(static_cast<int>(ufc->getComponent("weights").size()), inSz * outSz);
}

// Rejects DELTA learning rule
TEST(UnsupervisedFieldCouplingTest, RejectsDeltaLearningRule)
{
    auto ufc   = makeUFC("ufc", 50, 50, LearningRule::DELTA);
    auto inFld = makeField("in", 50);
    auto outFld = makeField("out", 50);
    simulation::Simulation sim("test", 0.1, 0.0, 0.0);
    sim.addElement(inFld);
    sim.addElement(ufc);
    sim.addElement(outFld);
    // addInput on a DELTA-rule UFC should log an error and NOT register the input
    ufc->addInput(inFld);
    EXPECT_TRUE(ufc->getInputs().empty());
}

// Clone
TEST(UnsupervisedFieldCouplingTest, CloneHasSameParameters)
{
    auto ufc = makeUFC("ufc", 40, 60, LearningRule::OJA);
    const auto cloned = std::dynamic_pointer_cast<UnsupervisedFieldCoupling>(ufc->clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), ufc->getParameters());
    EXPECT_EQ(cloned->getLabel(), ElementLabel::UNSUPERVISED_FIELD_COUPLING);
}

// Step output size
TEST(UnsupervisedFieldCouplingTest, StepOutputSizeUnchanged)
{
    const int outSz = 50;
    auto ufc    = makeUFC("ufc", outSz, outSz);
    auto inFld  = makeField("in",  outSz);
    auto outFld = makeField("out", outSz);

    simulation::Simulation sim("test", 0.1, 0.0, 0.0);
    sim.addElement(inFld);
    sim.addElement(ufc);
    sim.addElement(outFld);
    sim.createInteraction("in",  "output", "ufc");
    sim.createInteraction("ufc", "output", "out");
    sim.init();
    sim.step();

    EXPECT_EQ(static_cast<int>(ufc->getComponent("output").size()), outSz);
}

// Weights change when learning is active (Hebb)
TEST(UnsupervisedFieldCouplingTest, HebbWeightsUpdateWhenLearningActive)
{
    const int sz = 20;
    auto ufc    = makeUFC("ufc", sz, sz, LearningRule::HEBB);
    auto inFld  = makeField("in",  sz);
    auto outFld = makeField("out", sz);

    simulation::Simulation sim("test", 0.1, 0.0, 0.0);
    sim.addElement(inFld);
    sim.addElement(ufc);
    sim.addElement(outFld);
    sim.createInteraction("in",  "output", "ufc");
    sim.createInteraction("ufc", "output", "out");
    sim.init();

    // Inject a non-zero stimulus into the input field
    auto stim = std::make_shared<GaussStimulus>(
        ElementCommonParameters{"stim", ElementDimensions(sz, 1.0)},
        GaussStimulusParameters{5.0, 3.0, 10.0, false, false});
    sim.addElement(stim);
    sim.createInteraction("stim", "output", "in");

    ufc->setLearning(true);
    // Run a few steps so the fields develop activation
    for (int i = 0; i < 10; ++i) sim.step();

    const auto weights = ufc->getComponent("weights");
    const bool anyNonZero = std::any_of(weights.begin(), weights.end(),
        [](double w){ return std::abs(w) > 1e-12; });
    EXPECT_TRUE(anyNonZero);
}
