#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <fstream>
#include <string>

#include "simulation/simulation.h"
#include "simulation/simulation_file_manager.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/neural_field.h"
#include "elements/normal_noise.h"
#include "elements/activation_function.h"
#include "elements/boost_stimulus.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/field_coupling.h"
#include "elements/gauss_field_coupling.h"
#include "elements/memory_trace.h"
#include "exceptions/exception.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 100)
{
    const SigmoidFunction sigmoid{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 15.0, 50.0, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

static std::shared_ptr<GaussKernel> makeKernel(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    GaussKernelParameters gkp{ 5.0, 10.0, 0.0, true, true };
    return std::make_shared<GaussKernel>(cp, gkp);
}

static std::shared_ptr<NormalNoise> makeNoise(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    NormalNoiseParameters nnp{ 0.01 };
    return std::make_shared<NormalNoise>(cp, nnp);
}

static std::shared_ptr<MexicanHatKernel> makeMexHatKernel(const std::string& name, const int size = 100)
{
    ElementCommonParameters cp{ name, size };
    MexicanHatKernelParameters mhkp{ 3.0, 10.0, 6.0, 8.0, -0.05,
        true, true };
    return std::make_shared<MexicanHatKernel>(cp, mhkp);
}

// Fixture: creates and cleans up a temporary directory for file I/O tests.
class SimulationFileManagerTest : public ::testing::Test
{
protected:
    std::string tempDir;

    void SetUp() override
    {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        tempDir = (fs::temp_directory_path() / "dnf_sfm_tests" / info->name()).string();
        tempDir += "/";
        fs::create_directories(tempDir);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tempDir, ec);
    }
};

// ---------------------------------------------------------------------------
// SimulationFileManagerConstruction
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, ConstructionWithCustomPathDoesNotThrow)
{
    const auto sim = createSimulation("sfm-ctor", 1.0, 0.0, 0.0);
    EXPECT_NO_THROW(SimulationFileManager(sim, tempDir));
}

TEST_F(SimulationFileManagerTest, ConstructionWithEmptyPathDoesNotThrow)
{
    const auto sim = createSimulation("sfm-default-path", 1.0, 0.0, 0.0);
    EXPECT_NO_THROW(SimulationFileManager(sim, {}));
}

// ---------------------------------------------------------------------------
// SimulationFileManagerSave
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, SaveCreatesJsonFile)
{
    const auto sim = createSimulation("save-creates-file", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    const std::string expectedFile = tempDir + "save-creates-file.json";
    EXPECT_TRUE(fs::exists(expectedFile));
}

TEST_F(SimulationFileManagerTest, SavedFileIsValidJson)
{
    const auto sim = createSimulation("save-valid-json", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->addElement(makeStimulus("gs 1"));

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "save-valid-json.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());
    ASSERT_TRUE(parsed.contains("elements"));
    ASSERT_TRUE(parsed["elements"].is_array());
}

TEST_F(SimulationFileManagerTest, SavedFileContainsMetadataFields)
{
    const auto sim = createSimulation("my-sim-id", 2.5, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "my-sim-id.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());

    ASSERT_TRUE(parsed.contains("identifier"));
    ASSERT_TRUE(parsed.contains("deltaT"));
    EXPECT_EQ(parsed["identifier"].get<std::string>(), "my-sim-id");
    EXPECT_DOUBLE_EQ(parsed["deltaT"].get<double>(), 2.5);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesSimulationIdentifier)
{
    const auto simA = createSimulation("rt-identifier", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("placeholder", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-identifier.json" };
    sfmLoad.loadElementsFromJson();

    EXPECT_EQ(simB->getUniqueIdentifier(), "rt-identifier");
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesDeltaT)
{
    const auto simA = createSimulation("rt-deltat", 3.7, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("placeholder", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-deltat.json" };
    sfmLoad.loadElementsFromJson();

    EXPECT_DOUBLE_EQ(simB->getDeltaT(), 3.7);
}

TEST_F(SimulationFileManagerTest, SavedFileContainsAllElements)
{
    const auto sim = createSimulation("save-all-elements", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->addElement(makeStimulus("gs 1"));
    sim->addElement(makeKernel("gk 1"));
    sim->addElement(makeNoise("nn 1"));

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "save-all-elements.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());
    ASSERT_TRUE(parsed.contains("elements"));
    ASSERT_TRUE(parsed["elements"].is_array());

    EXPECT_EQ(static_cast<int>(parsed["elements"].size()), 4);
}

TEST_F(SimulationFileManagerTest, SavePreservesElementUniqueNames)
{
    const auto sim = createSimulation("save-names", 1.0, 0.0, 0.0);
    sim->addElement(makeField("my neural field"));
    sim->addElement(makeStimulus("my stimulus"));

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "save-names.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());
    ASSERT_TRUE(parsed.contains("elements"));
    ASSERT_TRUE(parsed["elements"].is_array());

    std::vector<std::string> names;
    for (const auto& el : parsed["elements"])
        names.push_back(el["uniqueName"].get<std::string>());

    EXPECT_NE(std::find(names.begin(), names.end(), "my neural field"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "my stimulus"), names.end());
}

TEST_F(SimulationFileManagerTest, SavePreservesInteractions)
{
    const auto sim = createSimulation("save-interactions", 1.0, 0.0, 0.0);
    const auto field = makeField("nf 1");
    const auto kernel = makeKernel("gk 1");
    sim->addElement(field);
    sim->addElement(kernel);
    sim->createInteraction("nf 1", "output", "gk 1");

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "save-interactions.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());
    ASSERT_TRUE(parsed.contains("elements"));
    ASSERT_TRUE(parsed["elements"].is_array());

    bool foundInteraction = false;
    for (const auto& el : parsed["elements"])
    {
        if (el["uniqueName"] == "gk 1" && !el["inputs"].empty())
        {
            for (const auto& input : el["inputs"])
            {
                if (input[0] == "nf 1" && input[1] == "output")
                    foundInteraction = true;
            }
        }
    }
    EXPECT_TRUE(foundInteraction);
}

TEST_F(SimulationFileManagerTest, SaveEmptySimulationCreatesEmptyArray)
{
    const auto sim = createSimulation("save-empty", 1.0, 0.0, 0.0);

    const SimulationFileManager sfm{ sim, tempDir };
    sfm.saveElementsToJson();

    std::ifstream file(tempDir + "save-empty.json");
    ASSERT_TRUE(file.is_open());
    nlohmann::json parsed;
    ASSERT_NO_THROW(file >> parsed);
    ASSERT_TRUE(parsed.is_object());
    ASSERT_TRUE(parsed.contains("elements"));
    ASSERT_TRUE(parsed["elements"].is_array());

    EXPECT_EQ(static_cast<int>(parsed["elements"].size()), 0);
}

// ---------------------------------------------------------------------------
// SimulationFileManagerLoad
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, LoadFromNonExistentFileDoesNotThrow)
{
    const auto sim = createSimulation("load-missing", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, tempDir + "does-not-exist.json" };
    EXPECT_NO_THROW(sfm.loadElementsFromJson());
}

TEST_F(SimulationFileManagerTest, LoadFromNonExistentFileLeavesSimulationEmpty)
{
    const auto sim = createSimulation("load-missing-empty", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, tempDir + "does-not-exist.json" };
    sfm.loadElementsFromJson();
    EXPECT_EQ(sim->getNumberOfElements(), 0);
}

TEST_F(SimulationFileManagerTest, LoadFromTestJsonCreatesCorrectElementCount)
{
    // test.json contains 13 elements
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/test.json";
    const auto sim = createSimulation("load-test", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();
    EXPECT_EQ(sim->getNumberOfElements(), 13);
}

TEST_F(SimulationFileManagerTest, LoadFromAndTestJsonCreatesCorrectElementCount)
{
    // and-test.json contains 10 elements
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-and-test", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();
    EXPECT_EQ(sim->getNumberOfElements(), 10);
}

TEST_F(SimulationFileManagerTest, LoadCreatesElementsWithCorrectNames)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-names", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    EXPECT_NO_THROW(sim->getElement("nf 1"));
    EXPECT_NO_THROW(sim->getElement("nf 2"));
    EXPECT_NO_THROW(sim->getElement("nf 3"));
    EXPECT_NO_THROW(sim->getElement("gk 1"));
    EXPECT_NO_THROW(sim->getElement("nn 1"));
}

TEST_F(SimulationFileManagerTest, LoadRestoresNeuralFieldParameters)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-nf-params", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    const auto nf = std::dynamic_pointer_cast<NeuralField>(sim->getElement("nf 1"));
    ASSERT_NE(nf, nullptr);
    const auto params = nf->getParameters();
    EXPECT_DOUBLE_EQ(params.tau, 25.0);
    EXPECT_DOUBLE_EQ(params.startingRestingLevel, -10.0);
}

TEST_F(SimulationFileManagerTest, LoadRestoresGaussKernelParameters)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-gk-params", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    const auto gk = std::dynamic_pointer_cast<GaussKernel>(sim->getElement("gk 1"));
    ASSERT_NE(gk, nullptr);
    const auto params = gk->getParameters();
    EXPECT_NEAR(params.amplitude, 3.3, 1e-4);
    EXPECT_NEAR(params.width, 3.0, 1e-6);
    EXPECT_NEAR(params.amplitudeGlobal, 0.0, 1e-6);
    EXPECT_TRUE(params.circular);
    EXPECT_TRUE(params.normalized);
}

TEST_F(SimulationFileManagerTest, LoadRestoresMexicanHatKernelParameters)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/test.json";
    const auto sim = createSimulation("load-mhk-params", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    const auto mhk = std::dynamic_pointer_cast<MexicanHatKernel>(sim->getElement("mhk 1"));
    ASSERT_NE(mhk, nullptr);
    const auto params = mhk->getParameters();
    EXPECT_NEAR(params.amplitudeExc, 21.355645810321416, 1e-6);
    EXPECT_NEAR(params.widthExc, 17.070753028665003, 1e-6);
    EXPECT_NEAR(params.amplitudeInh, 34.21938108533818, 1e-6);
    EXPECT_NEAR(params.widthInh, 17.441654765360106, 1e-6);
    EXPECT_NEAR(params.amplitudeGlobal, -0.015761297177952382, 1e-10);
    EXPECT_TRUE(params.circular);
    EXPECT_TRUE(params.normalized);
}

TEST_F(SimulationFileManagerTest, LoadRestoresNormalNoiseParameters)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-nn-params", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    const auto nn = std::dynamic_pointer_cast<NormalNoise>(sim->getElement("nn 1"));
    ASSERT_NE(nn, nullptr);
    EXPECT_NEAR(nn->getParameters().amplitude, 0.01, 1e-6);
}

TEST_F(SimulationFileManagerTest, LoadRestoresInteractions)
{
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    const auto sim = createSimulation("load-interactions", 1.0, 0.0, 0.0);
    const SimulationFileManager sfm{ sim, testFile };
    sfm.loadElementsFromJson();

    // nf 1 receives input from gk 1 and nn 1
    const auto inputs = sim->getElementsThatHaveSpecifiedElementAsInput("nf 1");
    EXPECT_FALSE(inputs.empty());
}

// ---------------------------------------------------------------------------
// SimulationSave (via Simulation::save)
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, SimulationSaveCreatesFile)
{
    const auto sim = createSimulation("sim-save", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    sim->save(tempDir);
    EXPECT_TRUE(fs::exists(tempDir + "sim-save.json"));
}

TEST_F(SimulationFileManagerTest, SimulationSaveWithDefaultPathCreatesFile)
{
    const auto sim = createSimulation("sim-save-default", 1.0, 0.0, 0.0);
    sim->addElement(makeField("nf 1"));
    EXPECT_NO_THROW(sim->save());
    const std::string defaultFile = std::string(OUTPUT_DIRECTORY) + "/simulations/sim-save-default.json";
    EXPECT_TRUE(fs::exists(defaultFile));
    fs::remove(defaultFile);
}

// ---------------------------------------------------------------------------
// SimulationRead (via Simulation::read)
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, SimulationReadFromTestJsonLoadsElements)
{
    const auto sim = createSimulation("sim-read", 1.0, 0.0, 0.0);
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/test.json";
    sim->read(testFile);
    EXPECT_EQ(sim->getNumberOfElements(), 13);
}

TEST_F(SimulationFileManagerTest, SimulationReadClearsExistingElementsBeforeLoading)
{
    const auto sim = createSimulation("sim-read-clean", 1.0, 0.0, 0.0);
    sim->addElement(makeField("pre-existing"));
    sim->addElement(makeStimulus("pre-existing-gs"));

    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    sim->read(testFile);

    EXPECT_EQ(sim->getNumberOfElements(), 10);
    EXPECT_EQ(sim->getElement("pre-existing"), nullptr);
}

TEST_F(SimulationFileManagerTest, SimulationReadInitializesSimulation)
{
    const auto sim = createSimulation("sim-read-init", 1.0, 0.0, 0.0);
    const std::string testFile = std::string(OUTPUT_DIRECTORY) + "/simulations/and-test.json";
    sim->read(testFile);
    EXPECT_TRUE(sim->isInitialized());
}

TEST_F(SimulationFileManagerTest, SimulationReadFromNonExistentFileDoesNotThrow)
{
    const auto sim = createSimulation("sim-read-missing", 1.0, 0.0, 0.0);
    EXPECT_NO_THROW(sim->read(tempDir + "no-such-file.json"));
}

// ---------------------------------------------------------------------------
// Round-trip: save then read back
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, RoundTripPreservesElementCount)
{
    const auto simA = createSimulation("rt-sim", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeKernel("gk 1"));
    simA->addElement(makeStimulus("gs 1"));
    simA->addElement(makeNoise("nn 1"));

    SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-loaded", 1.0, 0.0, 0.0);
    SimulationFileManager sfmLoad{ simB, tempDir + "rt-sim.json" };
    sfmLoad.loadElementsFromJson();

    EXPECT_EQ(simB->getNumberOfElements(), 4);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesElementNames)
{
    const auto simA = createSimulation("rt-names", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeStimulus("gs 1"));

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-loaded-names", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-names.json" };
    sfmLoad.loadElementsFromJson();

    EXPECT_NO_THROW(simB->getElement("nf 1"));
    EXPECT_NO_THROW(simB->getElement("gs 1"));
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesNeuralFieldParameters)
{
    const SigmoidFunction sigmoid{ 1.5, 8.0 };
    NeuralFieldParameters nfp{ 30.0, -7.5, sigmoid };
    ElementCommonParameters cp{ "nf rt", 100 };
    const auto field = std::make_shared<NeuralField>(cp, nfp);

    const auto simA = createSimulation("rt-nf-params", 1.0, 0.0, 0.0);
    simA->addElement(field);

    SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-nf-loaded", 1.0, 0.0, 0.0);
    SimulationFileManager sfmLoad{ simB, tempDir + "rt-nf-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loadedField = std::dynamic_pointer_cast<NeuralField>(simB->getElement("nf rt"));
    ASSERT_NE(loadedField, nullptr);
    const auto params = loadedField->getParameters();
    EXPECT_DOUBLE_EQ(params.tau, 30.0);
    EXPECT_DOUBLE_EQ(params.startingRestingLevel, -7.5);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesGaussKernelParameters)
{
    GaussKernelParameters gkp{ 4.5, 12.0, -0.02, true, false };
    ElementCommonParameters cp{ "gk rt", 100 };
    const auto kernel = std::make_shared<GaussKernel>(cp, gkp);

    const auto simA = createSimulation("rt-gk-params", 1.0, 0.0, 0.0);
    simA->addElement(kernel);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-gk-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-gk-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loadedKernel = std::dynamic_pointer_cast<GaussKernel>(simB->getElement("gk rt"));
    ASSERT_NE(loadedKernel, nullptr);
    EXPECT_EQ(loadedKernel->getParameters(), gkp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesMexicanHatKernelParameters)
{
    MexicanHatKernelParameters mhkp{ 3.0, 10.0, 6.0, 8.0, -0.05, true, true };
    ElementCommonParameters cp{ "mhk rt", 100 };
    const auto kernel = std::make_shared<MexicanHatKernel>(cp, mhkp);

    const auto simA = createSimulation("rt-mhk-params", 1.0, 0.0, 0.0);
    simA->addElement(kernel);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-mhk-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-mhk-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loadedKernel = std::dynamic_pointer_cast<MexicanHatKernel>(simB->getElement("mhk rt"));
    ASSERT_NE(loadedKernel, nullptr);
    EXPECT_EQ(loadedKernel->getParameters(), mhkp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesGaussStimulusParameters)
{
    GaussStimulusParameters gsp{ 6.0, 20.0, 35.0, false, true };
    ElementCommonParameters cp{ "gs rt", 100 };
    const auto stimulus = std::make_shared<GaussStimulus>(cp, gsp);

    const auto simA = createSimulation("rt-gs-params", 1.0, 0.0, 0.0);
    simA->addElement(stimulus);

    SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-gs-loaded", 1.0, 0.0, 0.0);
    SimulationFileManager sfmLoad{ simB, tempDir + "rt-gs-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loadedStimulus = std::dynamic_pointer_cast<GaussStimulus>(simB->getElement("gs rt"));
    ASSERT_NE(loadedStimulus, nullptr);
    const auto params = loadedStimulus->getParameters();
    EXPECT_NEAR(params.width, 6.0, 1e-6);
    EXPECT_NEAR(params.amplitude, 20.0, 1e-6);
    EXPECT_NEAR(params.position, 35.0, 1e-6);
    EXPECT_FALSE(params.circular);
    EXPECT_TRUE(params.normalized);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesNormalNoiseParameters)
{
    NormalNoiseParameters nnp{ 0.05 };
    ElementCommonParameters cp{ "nn rt", 100 };
    const auto noise = std::make_shared<NormalNoise>(cp, nnp);

    const auto simA = createSimulation("rt-nn-params", 1.0, 0.0, 0.0);
    simA->addElement(noise);

    SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-nn-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-nn-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loadedNoise = std::dynamic_pointer_cast<NormalNoise>(simB->getElement("nn rt"));
    ASSERT_NE(loadedNoise, nullptr);
    EXPECT_NEAR(loadedNoise->getParameters().amplitude, 0.05, 1e-6);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesInteractions)
{
    const auto simA = createSimulation("rt-interactions", 1.0, 0.0, 0.0);
    const auto field = makeField("nf 1");
    const auto kernel = makeKernel("gk 1");
    simA->addElement(field);
    simA->addElement(kernel);
    simA->createInteraction("nf 1", "output", "gk 1");

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-inter-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-interactions.json" };
    sfmLoad.loadElementsFromJson();

    // gk 1 should still have nf 1 as an input
    const auto consumers = simB->getElementsThatHaveSpecifiedElementAsInput("nf 1");
    ASSERT_FALSE(consumers.empty());
    EXPECT_EQ(consumers.front()->getUniqueName(), "gk 1");
}

TEST_F(SimulationFileManagerTest, SimulationSaveReadRoundTripViaSimulationMethods)
{
    const auto simA = createSimulation("sim-rt", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeKernel("gk 1"));
    simA->addElement(makeNoise("nn 1"));
    simA->createInteraction("nf 1", "output", "gk 1");
    simA->save(tempDir);

    const auto simB = createSimulation("sim-rt-b", 1.0, 0.0, 0.0);
    simB->read(tempDir + "sim-rt.json");

    EXPECT_EQ(simB->getNumberOfElements(), 3);
    EXPECT_NO_THROW(simB->getElement("nf 1"));
    EXPECT_NO_THROW(simB->getElement("gk 1"));
    EXPECT_NO_THROW(simB->getElement("nn 1"));
    EXPECT_TRUE(simB->isInitialized());
}

// ---------------------------------------------------------------------------
// Round-trip: element types not covered above
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, RoundTripPreservesBoostStimulusParameters)
{
    const BoostStimulusParameters bsp{ 7.5, false };
    const auto stimulus = std::make_shared<BoostStimulus>(
        ElementCommonParameters{ "bs rt", 100 }, bsp);

    const auto simA = createSimulation("rt-bs-params", 1.0, 0.0, 0.0);
    simA->addElement(stimulus);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-bs-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-bs-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<BoostStimulus>(simB->getElement("bs rt"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getParameters(), bsp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesOscillatoryKernelParameters)
{
    const OscillatoryKernelParameters okp{ 2.0, 0.12, 0.4, -0.02, true, false };
    const auto kernel = std::make_shared<OscillatoryKernel>(
        ElementCommonParameters{ "ok rt", 100 }, okp);

    const auto simA = createSimulation("rt-ok-params", 1.0, 0.0, 0.0);
    simA->addElement(kernel);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-ok-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-ok-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<OscillatoryKernel>(simB->getElement("ok rt"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getParameters(), okp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesAsymmetricGaussKernelParameters)
{
    const AsymmetricGaussKernelParameters agkp{ 4.0, 5.0, -0.01, 2.0, true, true };
    const auto kernel = std::make_shared<AsymmetricGaussKernel>(
        ElementCommonParameters{ "agk rt", 100 }, agkp);

    const auto simA = createSimulation("rt-agk-params", 1.0, 0.0, 0.0);
    simA->addElement(kernel);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-agk-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-agk-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<AsymmetricGaussKernel>(simB->getElement("agk rt"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getParameters(), agkp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesFieldCouplingParameters)
{
    const FieldCouplingParameters fcp{ {100, 1.0}, LearningRule::OJA, 2.0, 0.05 };
    const auto coupling = std::make_shared<FieldCoupling>(
        ElementCommonParameters{ "fc rt", 100 }, fcp);

    const auto simA = createSimulation("rt-fc-params", 1.0, 0.0, 0.0);
    simA->addElement(coupling);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-fc-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-fc-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<FieldCoupling>(simB->getElement("fc rt"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getParameters(), fcp);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesGaussFieldCouplingWithNoCouplings)
{
    const GaussFieldCouplingParameters gfcp{ {100, 1.0}, false, true, {} };
    const auto coupling = std::make_shared<GaussFieldCoupling>(
        ElementCommonParameters{ "gfc rt", 100 }, gfcp);

    const auto simA = createSimulation("rt-gfc-empty", 1.0, 0.0, 0.0);
    simA->addElement(coupling);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-gfc-empty-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-gfc-empty.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<GaussFieldCoupling>(simB->getElement("gfc rt"));
    ASSERT_NE(loaded, nullptr);
    const auto params = loaded->getParameters();
    EXPECT_EQ(params.circular,    gfcp.circular);
    EXPECT_EQ(params.normalized,  gfcp.normalized);
    EXPECT_EQ(params.inputFieldDimensions, gfcp.inputFieldDimensions);
    EXPECT_TRUE(params.couplings.empty());
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesGaussFieldCouplingWithCouplings)
{
    const std::vector<GaussCoupling> couplings = {
        GaussCoupling{ 25.0, 30.0, 3.0, 5.0 },
        GaussCoupling{ 50.0, 50.0, 4.0, 4.0 },
        GaussCoupling{ 75.0, 70.0, 2.0, 6.0 },
    };
    const GaussFieldCouplingParameters gfcp{ {100, 1.0}, true, false, couplings };
    const auto coupling = std::make_shared<GaussFieldCoupling>(
        ElementCommonParameters{ "gfc rt2", 100 }, gfcp);

    const auto simA = createSimulation("rt-gfc-couplings", 1.0, 0.0, 0.0);
    simA->addElement(coupling);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-gfc-couplings-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-gfc-couplings.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<GaussFieldCoupling>(simB->getElement("gfc rt2"));
    ASSERT_NE(loaded, nullptr);
    const auto params = loaded->getParameters();
    EXPECT_EQ(params.circular,   gfcp.circular);
    EXPECT_EQ(params.normalized, gfcp.normalized);
    ASSERT_EQ(params.couplings.size(), couplings.size());
    for (size_t i = 0; i < couplings.size(); ++i)
        EXPECT_EQ(params.couplings[i], couplings[i]);
}

TEST_F(SimulationFileManagerTest, RoundTripPreservesMemoryTraceParameters)
{
    const MemoryTraceParameters mtp{ 150.0, 800.0, 0.3 };
    const auto trace = std::make_shared<MemoryTrace>(
        ElementCommonParameters{ "mt rt", 100 }, mtp);

    const auto simA = createSimulation("rt-mt-params", 1.0, 0.0, 0.0);
    simA->addElement(trace);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-mt-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-mt-params.json" };
    sfmLoad.loadElementsFromJson();

    const auto loaded = std::dynamic_pointer_cast<MemoryTrace>(simB->getElement("mt rt"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getParameters(), mtp);
}

// ---------------------------------------------------------------------------
// All element types in a single simulation
// ---------------------------------------------------------------------------

TEST_F(SimulationFileManagerTest, RoundTripAllElementTypes)
{
    const auto simA = createSimulation("rt-all-elements", 1.0, 0.0, 0.0);
    simA->addElement(makeField("nf 1"));
    simA->addElement(makeStimulus("gs 1"));
    simA->addElement(std::make_shared<BoostStimulus>(
        ElementCommonParameters{ "bs 1", 100 }, BoostStimulusParameters{ 5.0, true }));
    simA->addElement(makeKernel("gk 1"));
    simA->addElement(makeMexHatKernel("mhk 1"));
    simA->addElement(std::make_shared<OscillatoryKernel>(
        ElementCommonParameters{ "ok 1", 100 },
        OscillatoryKernelParameters{ 1.0, 0.08, 0.3, -0.01, true, false }));
    simA->addElement(std::make_shared<AsymmetricGaussKernel>(
        ElementCommonParameters{ "agk 1", 100 },
        AsymmetricGaussKernelParameters{ 3.0, 3.0, 0.0, 0.0, true, true }));
    simA->addElement(makeNoise("nn 1"));
    simA->addElement(std::make_shared<FieldCoupling>(
        ElementCommonParameters{ "fc 1", 100 },
        FieldCouplingParameters{ {100, 1.0}, LearningRule::HEBB, 1.0, 0.01 }));
    simA->addElement(std::make_shared<GaussFieldCoupling>(
        ElementCommonParameters{ "gfc 1", 100 },
        GaussFieldCouplingParameters{ {100, 1.0}, true, false, {} }));
    simA->addElement(std::make_shared<MemoryTrace>(
        ElementCommonParameters{ "mt 1", 100 },
        MemoryTraceParameters{ 100.0, 1000.0, 0.5 }));

    constexpr int elementCount = 11;
    ASSERT_EQ(simA->getNumberOfElements(), elementCount);

    const SimulationFileManager sfmSave{ simA, tempDir };
    sfmSave.saveElementsToJson();

    const auto simB = createSimulation("rt-all-loaded", 1.0, 0.0, 0.0);
    const SimulationFileManager sfmLoad{ simB, tempDir + "rt-all-elements.json" };
    sfmLoad.loadElementsFromJson();

    EXPECT_EQ(simB->getNumberOfElements(), elementCount);
    EXPECT_NE(simB->getElement("nf 1"),  nullptr);
    EXPECT_NE(simB->getElement("gs 1"),  nullptr);
    EXPECT_NE(simB->getElement("bs 1"),  nullptr);
    EXPECT_NE(simB->getElement("gk 1"),  nullptr);
    EXPECT_NE(simB->getElement("mhk 1"), nullptr);
    EXPECT_NE(simB->getElement("ok 1"),  nullptr);
    EXPECT_NE(simB->getElement("agk 1"), nullptr);
    EXPECT_NE(simB->getElement("nn 1"),  nullptr);
    EXPECT_NE(simB->getElement("fc 1"),  nullptr);
    EXPECT_NE(simB->getElement("gfc 1"), nullptr);
    EXPECT_NE(simB->getElement("mt 1"),  nullptr);
}
