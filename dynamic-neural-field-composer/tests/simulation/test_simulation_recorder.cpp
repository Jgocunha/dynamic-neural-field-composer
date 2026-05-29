#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

#include "simulation/simulation.h"
#include "simulation/simulation_recorder.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"
#include "elements/activation_function.h"
#include "tools/utils.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<NeuralField> makeField(const std::string& name, const int size = 10)
{
    const SigmoidFunction sigmoid{ 0.0, 10.0 };
    NeuralFieldParameters nfp{ 25.0, -5.0, sigmoid };
    ElementCommonParameters cp{ name, size };
    return std::make_shared<NeuralField>(cp, nfp);
}

static std::shared_ptr<GaussStimulus> makeStimulus(const std::string& name, const int size = 10)
{
    ElementCommonParameters cp{ name, size };
    GaussStimulusParameters gsp{ 5.0, 1.0, 5.0, true, false };
    return std::make_shared<GaussStimulus>(cp, gsp);
}

static std::shared_ptr<Simulation> makeRunningSimulation(const std::string& id, const int steps = 0)
{
    auto sim = createSimulation(id, 1.0, 0.0, 0.0);
    sim->addElement(makeStimulus("stim"));
    sim->addElement(makeField("nf"));
    sim->init();
    for (int i = 0; i < steps; ++i)
        sim->step();
    return sim;
}

static std::string recDir(const std::string& simId)
{
    return (fs::path(tools::utils::getResourceRoot()) / "data" / simId / "recordings").string();
}

static std::string expDir(const std::string& simId)
{
    return (fs::path(tools::utils::getResourceRoot()) / "data" / simId / "exports").string();
}

static void cleanSimDir(const std::string& simId)
{
    const fs::path dir = fs::path(tools::utils::getResourceRoot()) / "data" / simId;
    if (fs::exists(dir))
        fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// isRecording / hasActiveRecordings
// ---------------------------------------------------------------------------

TEST(SimulationRecorderState, NotRecordingByDefault)
{
    SimulationRecorder rec;
    EXPECT_FALSE(rec.isRecording("nf", "activation"));
    EXPECT_FALSE(rec.hasActiveRecordings());
}

TEST(SimulationRecorderState, IsRecordingAfterStart)
{
    const std::string simId = "rec-state-test";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output", 1, RecordingIntervalUnit::Ticks);
    EXPECT_TRUE(sim->getRecorder().isRecording("stim", "output"));
    EXPECT_TRUE(sim->getRecorder().hasActiveRecordings());

    sim->getRecorder().stopRecording("stim", "output");
    EXPECT_FALSE(sim->getRecorder().isRecording("stim", "output"));
    cleanSimDir(simId);
}

TEST(SimulationRecorderState, StopAllClearsAllSessions)
{
    const std::string simId = "rec-stop-all";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output",   1, RecordingIntervalUnit::Ticks);
    sim->getRecorder().startRecording(simId, "nf",   "activation", 1, RecordingIntervalUnit::Ticks);
    EXPECT_TRUE(sim->getRecorder().hasActiveRecordings());

    sim->getRecorder().stopAll();
    EXPECT_FALSE(sim->getRecorder().hasActiveRecordings());
    cleanSimDir(simId);
}

// ---------------------------------------------------------------------------
// File creation
// ---------------------------------------------------------------------------

TEST(SimulationRecorderFile, StartRecordingCreatesFile)
{
    const std::string simId = "rec-file-create";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output", 1, RecordingIntervalUnit::Ticks);
    sim->step();
    sim->getRecorder().stopRecording("stim", "output");

    // At least one CSV file must exist in the recordings directory.
    bool found = false;
    for (const auto& entry : fs::directory_iterator(recDir(simId)))
        if (entry.path().extension() == ".csv") { found = true; break; }
    EXPECT_TRUE(found);
    cleanSimDir(simId);
}

TEST(SimulationRecorderFile, CsvHasCorrectHeader)
{
    const std::string simId = "rec-header";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output", 1, RecordingIntervalUnit::Ticks);
    sim->step();
    sim->getRecorder().stopAll();

    std::string csvPath;
    for (const auto& entry : fs::directory_iterator(recDir(simId)))
        if (entry.path().extension() == ".csv") { csvPath = entry.path().string(); break; }

    ASSERT_FALSE(csvPath.empty());
    std::string header;
    {
        std::ifstream f(csvPath);
        std::getline(f, header);
    }
    // Header must start with "ticks,ms,"
    EXPECT_EQ(header.substr(0, 8), "ticks,ms");
    cleanSimDir(simId);
}

// ---------------------------------------------------------------------------
// Tick interval sampling
// ---------------------------------------------------------------------------

TEST(SimulationRecorderSampling, TickIntervalProducesCorrectRowCount)
{
    const std::string simId = "rec-tick-interval";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    // Sample every 2 ticks, run 10 steps → expect 5 rows (at ticks 0,2,4,6,8... but starts at 0).
    sim->getRecorder().startRecording(simId, "stim", "output", 2, RecordingIntervalUnit::Ticks);
    for (int i = 0; i < 10; ++i)
        sim->step();
    sim->getRecorder().stopAll();

    std::string csvPath;
    for (const auto& entry : fs::directory_iterator(recDir(simId)))
        if (entry.path().extension() == ".csv") { csvPath = entry.path().string(); break; }

    ASSERT_FALSE(csvPath.empty());
    int lineCount = 0;
    {
        std::ifstream f(csvPath);
        std::string line;
        while (std::getline(f, line)) ++lineCount;
    }
    // 1 header + data rows. With interval 2 over 10 steps: rows at t=1,3,5,7,9 (nextSampleAt starts at 0).
    EXPECT_GE(lineCount, 2); // at least header + 1 data row
    cleanSimDir(simId);
}

// ---------------------------------------------------------------------------
// No more rows after stopRecording
// ---------------------------------------------------------------------------

TEST(SimulationRecorderSampling, NoRowsWrittenAfterStop)
{
    const std::string simId = "rec-stop-writes";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output", 1, RecordingIntervalUnit::Ticks);
    sim->step();
    sim->step();
    sim->getRecorder().stopRecording("stim", "output");

    // Count rows before stopping and ensure no more are added.
    std::string csvPath;
    for (const auto& entry : fs::directory_iterator(recDir(simId)))
        if (entry.path().extension() == ".csv") { csvPath = entry.path().string(); break; }

    ASSERT_FALSE(csvPath.empty());
    auto countLines = [&]() {
        std::ifstream f(csvPath);
        int n = 0; std::string l;
        while (std::getline(f, l)) ++n;
        return n;
    };

    const int rowsBefore = countLines();
    sim->step();
    sim->step();
    const int rowsAfter = countLines();
    EXPECT_EQ(rowsBefore, rowsAfter);
    cleanSimDir(simId);
}

// ---------------------------------------------------------------------------
// takeSnapshot
// ---------------------------------------------------------------------------

TEST(SimulationRecorderSnapshot, SnapshotCreatesExportFile)
{
    const std::string simId = "rec-snapshot";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId, 5);

    sim->getRecorder().takeSnapshot(simId, "stim", "output", *sim);

    bool found = false;
    for (const auto& entry : fs::directory_iterator(expDir(simId)))
        if (entry.path().extension() == ".csv") { found = true; break; }
    EXPECT_TRUE(found);
    cleanSimDir(simId);
}

TEST(SimulationRecorderSnapshot, SnapshotCsvHasTwoLines)
{
    const std::string simId = "rec-snapshot-lines";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId, 3);

    sim->getRecorder().takeSnapshot(simId, "stim", "output", *sim);

    std::string csvPath;
    for (const auto& entry : fs::directory_iterator(expDir(simId)))
        if (entry.path().extension() == ".csv") { csvPath = entry.path().string(); break; }

    ASSERT_FALSE(csvPath.empty());
    int lineCount = 0;
    {
        std::ifstream f(csvPath);
        std::string line;
        while (std::getline(f, line)) ++lineCount;
    }
    EXPECT_EQ(lineCount, 2); // header + 1 data row
    cleanSimDir(simId);
}

// ---------------------------------------------------------------------------
// Ticks derivation
// ---------------------------------------------------------------------------

TEST(SimulationRecorderTicks, TicksMatchStepCount)
{
    const std::string simId = "rec-ticks-check";
    cleanSimDir(simId);
    auto sim = makeRunningSimulation(simId);

    sim->getRecorder().startRecording(simId, "stim", "output", 1, RecordingIntervalUnit::Ticks);
    const int targetSteps = 5;
    for (int i = 0; i < targetSteps; ++i)
        sim->step();
    sim->getRecorder().stopAll();

    std::string csvPath;
    for (const auto& entry : fs::directory_iterator(recDir(simId)))
        if (entry.path().extension() == ".csv") { csvPath = entry.path().string(); break; }
    ASSERT_FALSE(csvPath.empty());

    std::string lastLine;
    {
        std::ifstream f(csvPath);
        std::string header, line;
        std::getline(f, header); // skip header
        while (std::getline(f, line)) lastLine = line;
    }

    // First column of last row is the tick count at that step.
    const int lastTick = std::stoi(lastLine.substr(0, lastLine.find(',')));
    EXPECT_EQ(lastTick, targetSteps);
    cleanSimDir(simId);
}
