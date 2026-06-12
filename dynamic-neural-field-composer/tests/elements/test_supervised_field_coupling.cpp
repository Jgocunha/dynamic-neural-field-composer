#include <gtest/gtest.h>
#include <memory>

#include "elements/supervised_field_coupling.h"
#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
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

static std::shared_ptr<SupervisedFieldCoupling> makeSFC(const std::string& name,
    int inSize = 50, int outSize = 50)
{
    ElementDimensions inDim{ inSize, 1.0 };
    SupervisedFieldCouplingParameters fcp{ inDim, 1.0, 0.01 };
    ElementCommonParameters cp{ name, ElementDimensions(outSize, 1.0) };
    return std::make_shared<SupervisedFieldCoupling>(cp, fcp);
}

// Construction
TEST(SupervisedFieldCouplingTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeSFC("sfc"));
}

TEST(SupervisedFieldCouplingTest, HasCorrectLabel)
{
    const auto sfc = makeSFC("sfc");
    EXPECT_EQ(sfc->getLabel(), ElementLabel::SUPERVISED_FIELD_COUPLING);
}

TEST(SupervisedFieldCouplingTest, LearningRuleIsAlwaysDelta)
{
    const auto sfc = makeSFC("sfc");
    EXPECT_EQ(sfc->getParameters().learningRule, LearningRule::DELTA);
}

TEST(SupervisedFieldCouplingTest, HasRequiredComponents)
{
    const auto sfc = makeSFC("sfc");
    EXPECT_NO_THROW(sfc->getComponent("input"));
    EXPECT_NO_THROW(sfc->getComponent("output"));
    EXPECT_NO_THROW(sfc->getComponent("weights"));
    EXPECT_NO_THROW(sfc->getComponent("reference"));
}

// Initialisation
TEST(SupervisedFieldCouplingTest, ComponentSizesAfterInit)
{
    const int inSz = 30, outSz = 50;
    auto sfc = makeSFC("sfc", inSz, outSz);
    sfc->init();
    EXPECT_EQ(static_cast<int>(sfc->getComponent("input").size()),     inSz);
    EXPECT_EQ(static_cast<int>(sfc->getComponent("output").size()),    outSz);
    EXPECT_EQ(static_cast<int>(sfc->getComponent("weights").size()),   inSz * outSz);
    EXPECT_EQ(static_cast<int>(sfc->getComponent("reference").size()), outSz);
}

// Reference source wiring
TEST(SupervisedFieldCouplingTest, ReferenceSourceNullByDefault)
{
    const auto sfc = makeSFC("sfc");
    EXPECT_EQ(sfc->getReferenceSource(), nullptr);
}

TEST(SupervisedFieldCouplingTest, ReferenceSourceSetCorrectly)
{
    auto sfc = makeSFC("sfc");
    auto ref = makeField("ref");
    sfc->addInput(ref, "reference");
    EXPECT_EQ(sfc->getReferenceSource(), ref);
    // reference is NOT in the standard inputs map
    EXPECT_TRUE(sfc->getInputs().empty());
}

// Clone
TEST(SupervisedFieldCouplingTest, CloneHasSameParameters)
{
    auto sfc = makeSFC("sfc", 30, 40);
    const auto cloned = std::dynamic_pointer_cast<SupervisedFieldCoupling>(sfc->clone());
    ASSERT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->getParameters(), sfc->getParameters());
    EXPECT_EQ(cloned->getLabel(), ElementLabel::SUPERVISED_FIELD_COUPLING);
}

// Step output size
TEST(SupervisedFieldCouplingTest, StepOutputSizeUnchanged)
{
    const int sz = 50;
    auto sfc    = makeSFC("sfc", sz, sz);
    auto inFld  = makeField("in",  sz);
    auto outFld = makeField("out", sz);
    auto refFld = makeField("ref", sz);

    simulation::Simulation sim("test", 0.1, 0.0, 0.0);
    sim.addElement(inFld);
    sim.addElement(sfc);
    sim.addElement(outFld);
    sim.addElement(refFld);
    sim.createInteraction("in",  "output", "sfc");
    sim.createInteraction("sfc", "output", "out");
    sfc->addInput(refFld, "reference");
    sim.init();
    sim.step();

    EXPECT_EQ(static_cast<int>(sfc->getComponent("output").size()), sz);
}

// Weights change when learning is active and reference != output
TEST(SupervisedFieldCouplingTest, DeltaWeightsUpdateWhenLearningActive)
{
    const int sz = 20;
    auto sfc    = makeSFC("sfc", sz, sz);
    auto inFld  = makeField("in",  sz);
    auto outFld = makeField("out", sz);
    auto refFld = makeField("ref", sz);

    simulation::Simulation sim("test", 0.1, 0.0, 0.0);
    sim.addElement(inFld);
    sim.addElement(sfc);
    sim.addElement(outFld);
    sim.addElement(refFld);
    sim.createInteraction("in",  "output", "sfc");
    sim.createInteraction("sfc", "output", "out");
    sfc->addInput(refFld, "reference");

    // Stimulate input and reference fields differently so error != 0
    auto stimIn = std::make_shared<GaussStimulus>(
        ElementCommonParameters{"stim_in", ElementDimensions(sz, 1.0)},
        GaussStimulusParameters{5.0, 3.0, 5.0, false, false});
    auto stimRef = std::make_shared<GaussStimulus>(
        ElementCommonParameters{"stim_ref", ElementDimensions(sz, 1.0)},
        GaussStimulusParameters{5.0, 3.0, 15.0, false, false});
    sim.addElement(stimIn);
    sim.addElement(stimRef);
    sim.createInteraction("stim_in",  "output", "in");
    sim.createInteraction("stim_ref", "output", "ref");

    sfc->setLearning(true);
    sim.init();
    for (int i = 0; i < 10; ++i) sim.step();

    const auto weights = sfc->getComponent("weights");
    const bool anyNonZero = std::any_of(weights.begin(), weights.end(),
        [](double w){ return std::abs(w) > 1e-12; });
    EXPECT_TRUE(anyNonZero);
}
