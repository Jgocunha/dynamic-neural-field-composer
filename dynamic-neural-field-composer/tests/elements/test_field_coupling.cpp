#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <fstream>
#include <algorithm>

#include "elements/field_coupling.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "simulation/simulation.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    const SigmoidFunction sig{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sig };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<FieldCoupling> makeFC(const std::string& name,
    const int inSize = 100, const int outSize = 100)
{
    ElementDimensions inDim{ inSize, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::HEBB, 1.0, 0.01 };
    ElementCommonParameters cp{ name, outSize };
    return std::make_shared<FieldCoupling>(cp, fcp);
}

// Fixture: provides a per-test temporary directory for weight file I/O.
class FieldCouplingFileTest : public ::testing::Test
{
protected:
    std::string tempDir;

    void SetUp() override
    {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        tempDir = (fs::temp_directory_path() / "dnf_fc_tests" / info->name()).string();
        fs::create_directories(tempDir);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tempDir, ec);
    }
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(FieldCouplingTest, ConstructionDoesNotThrow)
{
    EXPECT_NO_THROW(makeFC("fc 1"));
}

TEST(FieldCouplingTest, HasCorrectLabel)
{
    const auto fc = makeFC("fc 1");
    EXPECT_EQ(fc->getLabel(), ElementLabel::FIELD_COUPLING);
}

TEST(FieldCouplingTest, HasCorrectUniqueName)
{
    const auto fc = makeFC("my fc");
    EXPECT_EQ(fc->getUniqueName(), "my fc");
}

TEST(FieldCouplingTest, HasRequiredComponents)
{
    const auto fc = makeFC("fc 1");
    const auto list = fc->getComponentList();
    EXPECT_NE(std::find(list.begin(), list.end(), "input"),   list.end());
    EXPECT_NE(std::find(list.begin(), list.end(), "output"),  list.end());
    EXPECT_NE(std::find(list.begin(), list.end(), "weights"), list.end());
    EXPECT_NE(std::find(list.begin(), list.end(), "target"),  list.end());
}

TEST(FieldCouplingTest, TargetComponentIsSizedLikeOutput)
{
    const auto fc = makeFC("fc 1", 100, 60);
    EXPECT_EQ(fc->getComponentPtr("target")->size(), 60u);
}

// ---------------------------------------------------------------------------
// Parameters
// ---------------------------------------------------------------------------

TEST(FieldCouplingTest, GetParametersReflectsConstruction)
{
    ElementDimensions inDim{ 80, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::OJA, 2.0, 0.05 };
    ElementCommonParameters cp{ "fc 1", 60 };
    const auto fc = std::make_shared<FieldCoupling>(cp, fcp);

    const auto p = fc->getParameters();
    EXPECT_EQ(p.learningRule, LearningRule::OJA);
    EXPECT_DOUBLE_EQ(p.scalar, 2.0);
    EXPECT_DOUBLE_EQ(p.learningRate, 0.05);
    EXPECT_EQ(p.inputFieldDimensions.size, 80);
}

TEST(FieldCouplingTest, SetParametersUpdatesGetParameters)
{
    const auto fc = makeFC("fc 1");
    ElementDimensions inDim{ 50, 1.0 };
    FieldCouplingParameters newFcp{ inDim, LearningRule::DELTA, 3.0, 0.1 };
    fc->setParameters(newFcp);

    const auto p = fc->getParameters();
    EXPECT_EQ(p.learningRule, LearningRule::DELTA);
    EXPECT_DOUBLE_EQ(p.scalar, 3.0);
    EXPECT_DOUBLE_EQ(p.learningRate, 0.1);
}

TEST(FieldCouplingTest, SetLearningRateUpdatesParameter)
{
    const auto fc = makeFC("fc 1");
    fc->setLearningRate(0.42);
    EXPECT_DOUBLE_EQ(fc->getParameters().learningRate, 0.42);
}

TEST(FieldCouplingTest, SetLearningUpdatesFlag)
{
    const auto fc = makeFC("fc 1");
    fc->setLearning(true);
    EXPECT_TRUE(fc->getParameters().isLearningActive);
    fc->setLearning(false);
    EXPECT_FALSE(fc->getParameters().isLearningActive);
}

// ---------------------------------------------------------------------------
// toString / clone
// ---------------------------------------------------------------------------

TEST(FieldCouplingTest, ToStringReturnsNonEmptyString)
{
    const auto fc = makeFC("fc 1");
    EXPECT_FALSE(fc->toString().empty());
}

TEST(FieldCouplingTest, CloneCreatesIndependentCopy)
{
    const auto fc = makeFC("fc 1");
    fc->setLearningRate(0.77);
    const auto clone = fc->clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_NE(clone.get(), fc.get());
    const auto cloneFC = std::dynamic_pointer_cast<FieldCoupling>(clone);
    ASSERT_NE(cloneFC, nullptr);
    EXPECT_DOUBLE_EQ(cloneFC->getParameters().learningRate, 0.77);
    // Mutation of clone does not affect original
    cloneFC->setLearningRate(0.01);
    EXPECT_DOUBLE_EQ(fc->getParameters().learningRate, 0.77);
}

// ---------------------------------------------------------------------------
// Weights directory
// ---------------------------------------------------------------------------

TEST(FieldCouplingTest, SetGetWeightsDirectory)
{
    const auto fc = makeFC("fc 1");
    fc->setWeightsDirectory("/tmp/weights");
    EXPECT_EQ(fc->getWeightsDirectory(), "/tmp/weights");
}

// ---------------------------------------------------------------------------
// Weight file I/O
// ---------------------------------------------------------------------------

TEST_F(FieldCouplingFileTest, WriteWeightsCreatesFile)
{
    const auto fc = makeFC("write-test");
    fc->setWeightsDirectory(tempDir);
    fc->writeWeights();
    const std::string expected = tempDir + "/write-test_weights.txt";
    EXPECT_TRUE(fs::exists(expected));
}

TEST_F(FieldCouplingFileTest, ReadWeightsHandlesMissingFileGracefully)
{
    const auto fc = makeFC("no-file");
    fc->setWeightsDirectory(tempDir);
    EXPECT_NO_THROW(fc->readWeights());
}

TEST_F(FieldCouplingFileTest, WriteReadRoundTripProducesCorrectWeightCount)
{
    const int inSize = 10;
    const int outSize = 8;
    ElementDimensions inDim{ inSize, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::HEBB, 1.0, 0.01 };
    ElementCommonParameters cp{ "rt-fc", outSize };
    const auto fc = std::make_shared<FieldCoupling>(cp, fcp);
    fc->setWeightsDirectory(tempDir);

    fc->writeWeights();

    const std::string filename = tempDir + "/rt-fc_weights.txt";
    ASSERT_TRUE(fs::exists(filename));

    // Verify the file contains inSize * outSize whitespace-separated values
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());
    int count = 0;
    double val;
    while (file >> val)
        ++count;
    EXPECT_EQ(count, inSize * outSize);
}

TEST_F(FieldCouplingFileTest, ClearWeightsDoesNotThrow)
{
    const auto fc = makeFC("fc 1");
    EXPECT_NO_THROW(fc->clearWeights());
}

// ---------------------------------------------------------------------------
// Init / Step without connections — must not crash
// ---------------------------------------------------------------------------

TEST(FieldCouplingTest, InitWithoutConnectionsDoesNotCrash)
{
    const auto fc = makeFC("fc 1");
    EXPECT_NO_THROW(fc->init());
}

TEST(FieldCouplingTest, StepWithoutConnectionsDoesNotCrash)
{
    const auto fc = makeFC("fc 1");
    fc->init();
    EXPECT_NO_THROW(fc->step(1.0, 1.0));
}

// ---------------------------------------------------------------------------
// addInput — regression: input pointer must be refreshed immediately
// ---------------------------------------------------------------------------

// Regression: FieldCoupling::addInput() must refresh the internal input/output
// pointers so that learning can be enabled without calling init() again.
// Scenario: output connection exists at init() time; input is connected at runtime.
TEST(FieldCouplingTest, AddInputAtRuntimeDoesNotDisableLearning)
{
    const auto inputField  = makeField("if");
    const auto fc          = makeFC("fc", 100, 100);
    const auto outputField = makeField("of");

    // Output side is wired before init (simulates connection drawn before simulation start).
    outputField->addInput(fc);

    // init() sets fc->output (outputs map is already populated) but fc->input is null
    // because inputField is not yet connected.
    inputField->init();
    fc->init();
    outputField->init();

    // User draws the input connection at runtime (simulation already initialised).
    // Before the fix, addInput did not call updateInputField(), leaving fc->input null.
    fc->addInput(inputField);

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    // checkValidConnections() must find both pointers valid; learning must stay active.
    EXPECT_TRUE(fc->getParameters().isLearningActive);
}

// ---------------------------------------------------------------------------
// DELTA learning rule — supervised Widrow-Hoff. updateWeights() reads:
//   pre    = fc->input's "output" (g(u_in)), or whichever component the
//            input slot was wired from (see inputSourceComponent)
//   target = fc's own "target" component, fed by a second input pin
//   actual = fc's own "output" component (its own forward pass, scalar*W*input)
// and calls tools::math::deltaLearningRuleWidrowHoff(). The rule is NOT
// gated by the output field's own post-synaptic activity -- see
// DeltaLearnsWithOutputFieldAtRestingLevel below for why. HEBB/OJA are
// unaffected: they still read "activation" through normalize().
//
// These tests wire fields directly and bypass NeuralField::step() so hand-set
// "output"/"activation" values are not overwritten by field dynamics.
// ---------------------------------------------------------------------------

namespace
{
    // Wires inputField -> fc -> outputField the same way
    // FieldCouplingTest.AddInputAtRuntimeDoesNotDisableLearning does, without
    // ever stepping the NeuralFields (so their "activation"/"output" stay
    // whatever the test sets them to).
    void wireCoupling(const std::shared_ptr<NeuralField>& inputField,
        const std::shared_ptr<FieldCoupling>& fc,
        const std::shared_ptr<NeuralField>& outputField)
    {
        outputField->addInput(fc);
        inputField->init();
        fc->init();
        outputField->init();
        fc->addInput(inputField);
    }

    // Additionally wires targetField -> fc on the "target" slot.
    void wireCouplingWithTarget(const std::shared_ptr<NeuralField>& inputField,
        const std::shared_ptr<FieldCoupling>& fc,
        const std::shared_ptr<NeuralField>& outputField,
        const std::shared_ptr<NeuralField>& targetField)
    {
        wireCoupling(inputField, fc, outputField);
        targetField->init();
        fc->addInput(targetField, "target");
    }

    // Wires inputField -> fc -> outputField with the input slot declared as
    // "activation" instead of the default "output".
    void wireCouplingFromActivation(const std::shared_ptr<NeuralField>& inputField,
        const std::shared_ptr<FieldCoupling>& fc,
        const std::shared_ptr<NeuralField>& outputField)
    {
        outputField->addInput(fc);
        inputField->init();
        fc->init();
        outputField->init();
        fc->addInput(inputField, "activation");
    }
}

TEST(FieldCouplingDeltaLearningRule, TargetInputIsNotSummedIntoInput)
{
    // The critical routing test: a source connected on the "target" slot must
    // land only in components["target"], never accumulated into
    // components["input"] alongside the real input field.
    constexpr int size = 6;
    const auto inputField = makeField("route-in", size);
    const auto outputField = makeField("route-out", size);
    const auto targetField = makeField("route-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "route-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 2.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 9.0);

    fc->step(0.0, 0.1);

    const auto fcInput = *fc->getComponentPtr("input");
    const auto fcTarget = *fc->getComponentPtr("target");
    for (double v : fcInput)  EXPECT_DOUBLE_EQ(v, 2.0);
    for (double v : fcTarget) EXPECT_DOUBLE_EQ(v, 9.0);
}

TEST(FieldCouplingDeltaLearningRule, DeltaWithoutTargetDisablesLearning)
{
    constexpr int size = 5;
    const auto inputField = makeField("notarget-in", size);
    const auto outputField = makeField("notarget-out", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "notarget-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);
    fc->setLearning(true);
    fc->step(0.0, 0.1);

    EXPECT_FALSE(fc->getParameters().isLearningActive);
}

TEST(FieldCouplingDeltaLearningRule, HebbWithoutTargetKeepsLearningActive)
{
    // Negative control: the "DELTA requires a target" check must not affect
    // the other rules, which must keep working with only two connections.
    constexpr int size = 5;
    const auto inputField = makeField("hebbnotarget-in", size);
    const auto outputField = makeField("hebbnotarget-out", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::HEBB, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "hebbnotarget-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);
    fc->setLearning(true);
    fc->step(0.0, 0.1);

    EXPECT_TRUE(fc->getParameters().isLearningActive);
}

TEST(FieldCouplingDeltaLearningRule, DeltaZeroErrorLeavesWeightsUnchanged)
{
    // Hand-build weights so that scalar*W*input == target exactly (zero
    // error), then confirm a step leaves them untouched. This exercises the
    // scalar path *through* fc's own "output" (actual), without pre-scaling
    // pre a second time.
    constexpr int size = 4;
    constexpr double scalar = 2.0;
    const auto inputField = makeField("zeroerr-in", size);
    const auto outputField = makeField("zeroerr-out", size);
    const auto targetField = makeField("zeroerr-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, scalar, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "zeroerr-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::vector<double> in(size);
    for (int i = 0; i < size; ++i) in[i] = 0.5 + i;
    std::copy(in.begin(), in.end(), inputField->getComponentPtr("output")->begin());

    // Concentrate weight mass on index 0 so scalar*W*in == target exactly.
    std::vector<double> W(static_cast<std::size_t>(size) * size, 0.0);
    std::vector<double> target(size);
    for (int j = 0; j < size; ++j)
    {
        target[j] = 1.0 + j;
        W[j] = target[j] / (scalar * in[0]); // row 0 only
    }
    std::copy(target.begin(), target.end(), targetField->getComponentPtr("output")->begin());
    auto* weights = fc->getComponentPtr("weights");
    std::copy(W.begin(), W.end(), weights->begin());
    // Zero out every row but 0 so predicted[j] = scalar * W[0][j] * in[0].
    for (int i = 1; i < size; ++i)
        for (int j = 0; j < size; ++j)
            (*weights)[static_cast<std::size_t>(i) * size + j] = 0.0;
    for (int j = 0; j < size; ++j)
        (*weights)[j] = target[j] / (scalar * in[0]);
    const auto before = *weights;

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    const auto after = *fc->getComponentPtr("weights");
    ASSERT_EQ(before.size(), after.size());
    for (std::size_t k = 0; k < before.size(); ++k)
        EXPECT_NEAR(after[k], before[k], 1e-9);
}

TEST(FieldCouplingDeltaLearningRule, RemoveTargetClearsTargetBuffer)
{
    constexpr int size = 5;
    const auto inputField = makeField("removetarget-in", size);
    const auto outputField = makeField("removetarget-out", size);
    const auto targetField = makeField("removetarget-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "removetarget-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 5.0);
    fc->setLearning(true);
    fc->step(0.0, 0.1);
    ASSERT_TRUE(fc->getParameters().isLearningActive);

    fc->removeInput(targetField->getUniqueName());
    fc->step(0.0, 0.1);

    for (double v : *fc->getComponentPtr("target"))
        EXPECT_DOUBLE_EQ(v, 0.0);
    EXPECT_FALSE(fc->getParameters().isLearningActive);
}

TEST(FieldCouplingDeltaLearningRule, ChangeDimensionsResizesTarget)
{
    auto fc = makeFC("resize-fc", 100, 100);
    fc->changeDimensions(ElementDimensions{ 50, 1.0 });
    EXPECT_EQ(fc->getComponentPtr("target")->size(), 50u);
}

TEST(FieldCouplingDeltaLearningRule, DeltaLearnsWithOutputFieldAtRestingLevel)
{
    // Regression: a coupling's output field's ONLY drive is the coupling
    // itself. With weights starting at zero, the coupling's forward pass is
    // zero, so the output field never leaves its resting level and its own
    // "output" (g(u_out)) stays ~0 forever. A rule gated by that value can
    // never bootstrap -- see tools::math test PostSynapticActivityIsNotRequired.
    // This must NOT be the case: the error term alone must be enough to
    // start growing weights.
    constexpr int size = 4;
    const auto inputField = makeField("bootstrap-in", size);
    const auto outputField = makeField("bootstrap-out", size);
    const auto targetField = makeField("bootstrap-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "bootstrap-fc", size }, fcp);

    // wireCouplingWithTarget leaves outputField's "output" at whatever init()
    // set it to (resting level, never driven) -- exactly the scenario above.
    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 1.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 5.0);

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    for (double w : *fc->getComponentPtr("weights"))
        EXPECT_GT(w, 0.0);
}

TEST(FieldCouplingDeltaLearningRule, DeltaConvergesToTarget)
{
    constexpr int size = 8;
    const auto inputField = makeField("conv-in", size);
    const auto outputField = makeField("conv-out", size);
    const auto targetField = makeField("conv-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.05, 0.0 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "conv-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::vector<double> in(size), target(size);
    for (int i = 0; i < size; ++i) { in[i] = 0.3 + 0.1 * i; target[i] = 1.0 + 0.5 * i; }
    std::copy(in.begin(), in.end(), inputField->getComponentPtr("output")->begin());
    std::copy(target.begin(), target.end(), targetField->getComponentPtr("output")->begin());

    fc->setLearning(true);
    for (int step = 0; step < 2000; ++step)
        fc->step(static_cast<double>(step) * 0.1, 0.1);

    const auto actual = *fc->getComponentPtr("output");
    double maxAbsError = 0.0;
    for (int j = 0; j < size; ++j)
        maxAbsError = std::max(maxAbsError, std::abs(target[j] - actual[j]));
    EXPECT_LT(maxAbsError, 1e-3);
}

TEST(FieldCouplingDeltaLearningRule, SimulationInitWithTargetConnectionDoesNotThrow)
{
    // Regression: Simulation::init() calls element->buildInputCache() on every
    // element unconditionally. The base Element::buildInputCache() does
    // elem->components.at(compName) for every (source, declaredComponent) pair in
    // `inputs` -- for a source declared on the "target" slot, compName is "target",
    // which no NeuralField has, so the base implementation throws
    // std::out_of_range. FieldCoupling must override buildInputCache() (as a
    // no-op, since its updateInput() override reads `inputs` directly and never
    // uses the cache) so this path is safe.
    constexpr int size = 5;
    auto simulation = std::make_shared<Simulation>("sim-init-target", 1.0, 0.0, 0.0);

    const auto inputField = makeField("siminit-in", size);
    const auto outputField = makeField("siminit-out", size);
    const auto targetField = makeField("siminit-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "siminit-fc", size }, fcp);

    simulation->addElement(inputField);
    simulation->addElement(outputField);
    simulation->addElement(targetField);
    simulation->addElement(fc);

    fc->addInput(inputField);
    outputField->addInput(fc);
    fc->addInput(targetField, "target");

    EXPECT_NO_THROW(simulation->init());
    EXPECT_NO_THROW(simulation->step());
}

// ---------------------------------------------------------------------------
// Activation-pin wiring: a coupling's input and target slots may be declared
// as "activation" instead of the default "output", so the forward pass and
// (for DELTA) the learning rule read whichever component the user actually
// wired in the node graph -- see FieldCoupling::addInput()/parseSlot().
// ---------------------------------------------------------------------------

TEST(FieldCouplingActivationWiring, ActivationWiredInputFeedsForwardPass)
{
    constexpr int size = 4;
    const auto inputField = makeField("actin-in", size);
    const auto outputField = makeField("actin-out", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::HEBB, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "actin-fc", size }, fcp);

    wireCouplingFromActivation(inputField, fc, outputField);

    std::fill(inputField->getComponentPtr("activation")->begin(), inputField->getComponentPtr("activation")->end(), 3.0);
    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 7.0);

    fc->step(0.0, 0.1);

    for (double v : *fc->getComponentPtr("input"))
        EXPECT_DOUBLE_EQ(v, 3.0);
}

TEST(FieldCouplingActivationWiring, DeltaRuleReadsActivationWhenWiredFromActivationPin)
{
    constexpr int size = 3;
    const auto inputField = makeField("actdelta-in", size);
    const auto outputField = makeField("actdelta-out", size);
    const auto targetField = makeField("actdelta-target", size);
    constexpr double learningRate = 0.1;
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, learningRate };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "actdelta-fc", size }, fcp);

    outputField->addInput(fc);
    inputField->init();
    fc->init();
    outputField->init();
    fc->addInput(inputField, "activation");
    targetField->init();
    fc->addInput(targetField, "target");

    std::fill(inputField->getComponentPtr("activation")->begin(), inputField->getComponentPtr("activation")->end(), 4.0);
    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 999.0);
    std::fill(outputField->getComponentPtr("output")->begin(), outputField->getComponentPtr("output")->end(), 1.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 2.0);
    auto* weights = fc->getComponentPtr("weights");
    std::fill(weights->begin(), weights->end(), 0.0);

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    // actual = scalar * W * pre = 0 (weights start at zero) => err = target - actual = 2.0
    // Δw = lr * pre * post * err = 0.1 * 4.0 * 1.0 * 2.0 = 0.8 for every weight,
    // which is only reachable if pre came from "activation" (4.0), not "output" (999.0).
    for (double w : *fc->getComponentPtr("weights"))
        EXPECT_NEAR(w, 0.8, 1e-9);
}

TEST(FieldCouplingActivationWiring, DeltaRuleStillReadsOutputWhenWiredFromOutputPin)
{
    constexpr int size = 3;
    const auto inputField = makeField("outdelta-in", size);
    const auto outputField = makeField("outdelta-out", size);
    const auto targetField = makeField("outdelta-target", size);
    constexpr double learningRate = 0.1;
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, learningRate };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "outdelta-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::fill(inputField->getComponentPtr("activation")->begin(), inputField->getComponentPtr("activation")->end(), 999.0);
    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 4.0);
    std::fill(outputField->getComponentPtr("output")->begin(), outputField->getComponentPtr("output")->end(), 1.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 2.0);
    auto* weights = fc->getComponentPtr("weights");
    std::fill(weights->begin(), weights->end(), 0.0);

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    for (double w : *fc->getComponentPtr("weights"))
        EXPECT_NEAR(w, 0.8, 1e-9);
}

TEST(FieldCouplingActivationWiring, TargetActivationSlotReadsActivation)
{
    constexpr int size = 4;
    const auto inputField = makeField("tgtact-in", size);
    const auto outputField = makeField("tgtact-out", size);
    const auto targetField = makeField("tgtact-target", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "tgtact-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);
    targetField->init();
    fc->addInput(targetField, "target:activation");

    std::fill(targetField->getComponentPtr("activation")->begin(), targetField->getComponentPtr("activation")->end(), 6.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 999.0);

    fc->step(0.0, 0.1);

    for (double v : *fc->getComponentPtr("target"))
        EXPECT_DOUBLE_EQ(v, 6.0);
}

TEST(FieldCouplingActivationWiring, TargetSlotDefaultStillReadsOutput)
{
    constexpr int size = 4;
    const auto inputField = makeField("tgtout-in", size);
    const auto outputField = makeField("tgtout-out", size);
    const auto targetField = makeField("tgtout-target", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "tgtout-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::fill(targetField->getComponentPtr("activation")->begin(), targetField->getComponentPtr("activation")->end(), 999.0);
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(), 6.0);

    fc->step(0.0, 0.1);

    for (double v : *fc->getComponentPtr("target"))
        EXPECT_DOUBLE_EQ(v, 6.0);
}

TEST(FieldCouplingActivationWiring, ActivationTargetIsNotSummedIntoInput)
{
    constexpr int size = 5;
    const auto inputField = makeField("actroute-in", size);
    const auto outputField = makeField("actroute-out", size);
    const auto targetField = makeField("actroute-target", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "actroute-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);
    targetField->init();
    fc->addInput(targetField, "target:activation");

    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 2.0);
    std::fill(targetField->getComponentPtr("activation")->begin(), targetField->getComponentPtr("activation")->end(), 9.0);

    fc->step(0.0, 0.1);

    for (double v : *fc->getComponentPtr("input"))
        EXPECT_DOUBLE_EQ(v, 2.0);
    for (double v : *fc->getComponentPtr("target"))
        EXPECT_DOUBLE_EQ(v, 9.0);
}

TEST(FieldCouplingActivationWiring, TargetActivationCountsAsTargetNotInput)
{
    // Regression: updateInputField()'s "skip target-slot entries" logic must
    // also skip "target:activation", or a coupling with one real input and
    // one target:activation connection would see inputCount == 2 and refuse
    // to resolve fc->input at all.
    constexpr int size = 5;
    const auto inputField = makeField("counttgt-in", size);
    const auto outputField = makeField("counttgt-out", size);
    const auto targetField = makeField("counttgt-target", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "counttgt-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);
    targetField->init();
    fc->addInput(targetField, "target:activation");

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    EXPECT_TRUE(fc->getParameters().isLearningActive);
}

TEST(FieldCouplingActivationWiring, HebbUnaffectedByActivationWiring)
{
    // HEBB/OJA must keep reading "activation" unconditionally, regardless of
    // which pin (Output or Activation) the input/output fields were wired
    // from -- this pins the deliberate non-change.
    constexpr int size = 4;
    const auto makeAndStep = [&](const std::string& inputComponent) {
        const auto inputField = makeField("hebbwire-in-" + inputComponent, size);
        const auto outputField = makeField("hebbwire-out-" + inputComponent, size);
        FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::HEBB, 1.0, 0.1 };
        const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "hebbwire-fc-" + inputComponent, size }, fcp);

        outputField->addInput(fc);
        inputField->init();
        fc->init();
        outputField->init();
        fc->addInput(inputField, inputComponent);

        std::fill(inputField->getComponentPtr("activation")->begin(), inputField->getComponentPtr("activation")->end(), 3.0);
        std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 999.0);
        std::fill(outputField->getComponentPtr("activation")->begin(), outputField->getComponentPtr("activation")->end(), 1.5);

        fc->setLearning(true);
        fc->step(0.0, 0.1);
        return *fc->getComponentPtr("weights");
        };

    const auto viaOutput = makeAndStep("output");
    const auto viaActivation = makeAndStep("activation");

    ASSERT_EQ(viaOutput.size(), viaActivation.size());
    for (std::size_t i = 0; i < viaOutput.size(); ++i)
        EXPECT_DOUBLE_EQ(viaOutput[i], viaActivation[i]);
}

TEST(FieldCouplingActivationWiring, SimulationInitWithActivationTargetDoesNotThrow)
{
    constexpr int size = 5;
    auto simulation = std::make_shared<Simulation>("sim-init-activation-target", 1.0, 0.0, 0.0);

    const auto inputField = makeField("siminitact-in", size);
    const auto outputField = makeField("siminitact-out", size);
    const auto targetField = makeField("siminitact-target", size);
    FieldCouplingParameters fcp{ ElementDimensions{ size, 1.0 }, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "siminitact-fc", size }, fcp);

    simulation->addElement(inputField);
    simulation->addElement(outputField);
    simulation->addElement(targetField);
    simulation->addElement(fc);

    fc->addInput(inputField, "activation");
    outputField->addInput(fc);
    fc->addInput(targetField, "target:activation");

    EXPECT_NO_THROW(simulation->init());
    EXPECT_NO_THROW(simulation->step());
}

TEST(FieldCouplingDeltaLearningRule, DecayShrinksWeightNormEvenWithZeroError)
{
    constexpr int size = 4;
    const auto inputField = makeField("decay-in", size);
    const auto outputField = makeField("decay-out", size);
    const auto targetField = makeField("decay-target", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1, /*decayRate=*/0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "decay-fc", size }, fcp);

    wireCouplingWithTarget(inputField, fc, outputField, targetField);

    std::fill(inputField->getComponentPtr("output")->begin(), inputField->getComponentPtr("output")->end(), 1.0);
    std::fill(outputField->getComponentPtr("output")->begin(), outputField->getComponentPtr("output")->end(), 1.0);
    auto* weights = fc->getComponentPtr("weights");
    std::fill(weights->begin(), weights->end(), 1.0);
    // With W all 1 and input all 1, actual == target when target == size, so
    // set the target to match (zero error) and let decay alone act.
    std::fill(targetField->getComponentPtr("output")->begin(), targetField->getComponentPtr("output")->end(),
        static_cast<double>(size));

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    for (double w : *fc->getComponentPtr("weights"))
        EXPECT_LT(w, 1.0);
}

// ---------------------------------------------------------------------------
// FieldCouplingParameters equality / toString
// ---------------------------------------------------------------------------

TEST(FieldCouplingParametersTest, EqualParametersCompareEqual)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters a{ dim, LearningRule::HEBB, 1.0, 0.01 };
    const FieldCouplingParameters b{ dim, LearningRule::HEBB, 1.0, 0.01 };
    EXPECT_EQ(a, b);
}

TEST(FieldCouplingParametersTest, DifferentLearningRuleComparesNotEqual)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters a{ dim, LearningRule::HEBB, 1.0, 0.01 };
    const FieldCouplingParameters b{ dim, LearningRule::OJA,  1.0, 0.01 };
    EXPECT_NE(a, b);
}

TEST(FieldCouplingParametersTest, DifferentScalarComparesNotEqual)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters a{ dim, LearningRule::HEBB, 1.0, 0.01 };
    const FieldCouplingParameters b{ dim, LearningRule::HEBB, 2.0, 0.01 };
    EXPECT_NE(a, b);
}

TEST(FieldCouplingParametersTest, ToStringReturnsNonEmpty)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters fcp{ dim, LearningRule::HEBB, 1.0, 0.01 };
    EXPECT_FALSE(fcp.toString().empty());
}

TEST(FieldCouplingParametersTest, DecayRateDefaultsToZero)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters fcp{ dim, LearningRule::DELTA, 1.0, 0.01 };
    EXPECT_DOUBLE_EQ(fcp.decayRate, 0.0);
}

TEST(FieldCouplingParametersTest, DifferentDecayRateComparesNotEqual)
{
    ElementDimensions dim{ 100, 1.0 };
    const FieldCouplingParameters a{ dim, LearningRule::DELTA, 1.0, 0.01, 0.0 };
    const FieldCouplingParameters b{ dim, LearningRule::DELTA, 1.0, 0.01, 0.05 };
    EXPECT_NE(a, b);
}

TEST(FieldCouplingTest, SetDecayRateUpdatesParameter)
{
    const auto fc = makeFC("fc 1");
    fc->setDecayRate(0.15);
    EXPECT_DOUBLE_EQ(fc->getParameters().decayRate, 0.15);
}
