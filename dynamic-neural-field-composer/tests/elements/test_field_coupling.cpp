#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <fstream>

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
