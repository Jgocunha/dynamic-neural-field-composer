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
// DELTA learning rule (issue #38) — updateWeights() reads fc->input's and
// fc->output's own "activation" components (the connected NeuralFields, not
// FieldCoupling's internal "input"/"output"), normalizes them, and calls
// tools::math::unsupervisedDeltaLearningRule(). These tests wire that up
// directly, bypassing NeuralField::step() so the hand-set activations are
// not overwritten by field dynamics.
// ---------------------------------------------------------------------------

namespace
{
    // Wires inputField -> fc -> outputField the same way
    // FieldCouplingTest.AddInputAtRuntimeDoesNotDisableLearning does, without
    // ever stepping the NeuralFields (so their "activation" stays whatever the
    // test sets it to).
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
}

TEST(FieldCouplingDeltaLearningRule, ZeroErrorProducesNoWeightChange)
{
    constexpr int size = 10;
    const auto inputField = makeField("delta-in", size);
    const auto outputField = makeField("delta-out", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "delta-fc", size }, fcp);

    wireCoupling(inputField, fc, outputField);

    // Weights start at zero (constructor default), so the coupling's
    // prediction w^T*input is zero regardless of the input activation.
    const auto originalWeights = *fc->getComponentPtr("weights");
    for (double w : originalWeights)
        EXPECT_DOUBLE_EQ(w, 0.0);

    // A uniform output activation normalizes to an all-zero target (every
    // entry is the vector's minimum, which normalize() maps to 0), so the
    // error (target - predicted) is exactly zero everywhere.
    auto* outActivation = outputField->getComponentPtr("activation");
    std::fill(outActivation->begin(), outActivation->end(), 3.0);
    auto* inActivation = inputField->getComponentPtr("activation");
    for (std::size_t i = 0; i < inActivation->size(); ++i)
        (*inActivation)[i] = static_cast<double>(i) * 0.37;

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    const auto updatedWeights = *fc->getComponentPtr("weights");
    EXPECT_EQ(updatedWeights, originalWeights);
}

TEST(FieldCouplingDeltaLearningRule, NonUniformTargetIncreasesWeightsFromZero)
{
    constexpr int size = 10;
    const auto inputField = makeField("delta-in2", size);
    const auto outputField = makeField("delta-out2", size);
    ElementDimensions inDim{ size, 1.0 };
    FieldCouplingParameters fcp{ inDim, LearningRule::DELTA, 1.0, 0.1 };
    const auto fc = std::make_shared<FieldCoupling>(ElementCommonParameters{ "delta-fc2", size }, fcp);

    wireCoupling(inputField, fc, outputField);

    // Weights start at zero, so predicted = 0 and error = normalize(output).
    // A non-uniform output activation normalizes to a non-negative vector
    // that is not all-zero (normalize() preserves relative order — see
    // NormalizeVector.PreservesRelativeOrder in test_math.cpp).
    auto* outActivation = outputField->getComponentPtr("activation");
    for (std::size_t i = 0; i < outActivation->size(); ++i)
        (*outActivation)[i] = static_cast<double>(i) * 1.5;
    auto* inActivation = inputField->getComponentPtr("activation");
    for (std::size_t i = 0; i < inActivation->size(); ++i)
        (*inActivation)[i] = static_cast<double>(i) * 0.37 + 1.0;

    fc->setLearning(true);
    fc->step(0.0, 0.1);

    const auto updatedWeights = *fc->getComponentPtr("weights");

    // Both input and error are non-negative (normalize()'s range), so no
    // weight should have moved below zero, and since neither vector is
    // all-zero, at least one weight must have strictly increased.
    bool anyIncreased = false;
    for (double w : updatedWeights)
    {
        EXPECT_GE(w, 0.0);
        if (w > 1e-9) anyIncreased = true;
    }
    EXPECT_TRUE(anyIncreased);
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
