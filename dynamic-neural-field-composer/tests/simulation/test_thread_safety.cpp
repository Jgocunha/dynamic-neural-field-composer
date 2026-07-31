// Thread-safety regression tests for the multi-Simulation usage model.
//
// dnf-composer supports running INDEPENDENT Simulation objects on separate
// threads (one Simulation per thread, never shared) — the model used by
// consumers like neat-dnfs, which evaluates a population of phenotypes in
// parallel. These tests guard the pieces of global state that were hardened
// for that contract:
//   1. Simulation::generateUniqueIdentifier() racing on std::localtime's
//      shared static buffer (.claude/reports/dnf_composer-tsan-data-races.md),
//   2. getResourceRoot()'s magic-static initialization racing with itself,
//   3. element/plot uniqueIdentifierCounter (atomic — unique IDs under
//      concurrent construction),
//   4. the logger (no shared mutable global; serialized sinks; atomic level),
//   5. that stepping independent Simulations in parallel matches serial output.
//
// These are stress tests: they run many iterations to surface races. They do
// NOT exercise sharing a single Simulation across threads (unsupported).

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "simulation/simulation.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/neural_field.h"
#include "elements/normal_noise.h"
#include "elements/activation_function.h"
#include "tools/logger.h"
#include "tools/utils.h"
#include "scoped_min_log_level.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

namespace
{
    // Canonical 1D detection sim (stimulus + field + lateral kernel + amp-0 noise),
    // matching the benchmark/profiler setup. amp-0 noise => deterministic output.
    std::shared_ptr<Simulation> buildDetectionSim()
    {
        auto sim = std::make_shared<Simulation>("ts", 25.0, 0.0, 0.0);
        constexpr int size = 100;
        auto stim = std::make_shared<GaussStimulus>(
            ElementCommonParameters{ "gauss stimulus", size },
            GaussStimulusParameters{ 5.0, 10.0, 50.0, true, false });
        auto field = std::make_shared<NeuralField>(
            ElementCommonParameters{ "neural field u", size },
            NeuralFieldParameters{ 25.0, -5.0, SigmoidFunction{ 0.0, 100.0 } });
        auto kernel = std::make_shared<GaussKernel>(
            ElementCommonParameters{ "gauss kernel", size },
            GaussKernelParameters{ 3.0, 5.0, 0.0, true, true });
        auto noise = std::make_shared<NormalNoise>(
            ElementCommonParameters{ "normal noise", size }, NormalNoiseParameters{ 0.0 });
        sim->addElement(stim); sim->addElement(field); sim->addElement(kernel); sim->addElement(noise);
        field->addInput(stim); field->addInput(noise); kernel->addInput(field); field->addInput(kernel);
        return sim;
    }

    std::vector<double> runSerial(int steps)
    {
        auto sim = buildDetectionSim();
        sim->init();
        for (int i = 0; i < steps; ++i) sim->step();
        return sim->getComponent("neural field u", "activation");
    }
}

// 1. Simulation::generateUniqueIdentifier() racing on std::localtime's shared
//    static buffer.
TEST(ThreadSafety, ConcurrentSimulationConstructionDoesNotRace)
{
    constexpr int kThreads = 8;
    constexpr int kIterationsPerThread = 20;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([]()
        {
            for (int j = 0; j < kIterationsPerThread; ++j)
            {
                Simulation sim("", 1.0, 0.0, 0.0);
                EXPECT_FALSE(sim.getUniqueIdentifier().empty());
            }
        });
    }

    for (auto& t : threads)
        t.join();
}

// 2. getResourceRoot()'s magic-static initialization racing with itself.
TEST(ThreadSafety, ConcurrentGetResourceRootDoesNotRace)
{
    constexpr int kThreads = 8;

    std::vector<std::thread> threads;
    std::vector<std::string> results(kThreads);
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([i, &results]()
        {
            results[i] = tools::utils::getResourceRoot();
        });
    }

    for (auto& t : threads)
        t.join();

    for (const auto& result : results)
        EXPECT_EQ(result, results.front());
}

// 3. Unique IDs under concurrent element construction.
//    With a non-atomic counter this fails intermittently (duplicate/skipped IDs).
TEST(ThreadSafety, UniqueIdsUnderConcurrentConstruction)
{
    constexpr int kThreads = 8;
    constexpr int kPerThread = 500;

    std::vector<std::thread> threads;
    std::vector<std::vector<int>> ids(kThreads);
    for (int t = 0; t < kThreads; ++t)
    {
        ids[t].reserve(kPerThread);
        threads.emplace_back([&ids, t]() {
            for (int i = 0; i < kPerThread; ++i)
            {
                NeuralField nf{ ElementCommonParameters{ "f", 10 },
                                NeuralFieldParameters{ 25.0, -5.0, SigmoidFunction{ 0.0, 10.0 } } };
                ids[t].push_back(nf.getUniqueIdentifier());
            }
        });
    }
    for (auto& th : threads) th.join();

    std::set<int> all;
    int total = 0;
    for (const auto& v : ids)
        for (int id : v) { all.insert(id); ++total; }

    EXPECT_EQ(static_cast<int>(all.size()), total)
        << "duplicate uniqueIdentifiers assigned under concurrent construction";
}

// 4. Concurrent logging + setMinLogLevel must not crash or corrupt.
//    CONSOLE mode avoids the GUI sink; we just assert it survives a hammering.
TEST(ThreadSafety, ConcurrentLoggingDoesNotCrash)
{
    using namespace dnf_composer::tools::logger;
    // Keep the test quiet (FATAL still emits) WITHOUT leaking the raised threshold
    // into later suites -- the guard restores the previous value on scope exit.
    const dnf_composer::test::ScopedMinLogLevel quiet{ LogLevel::FATAL };

    constexpr int kWriters = 6;
    constexpr int kIters = 2000;
    std::atomic<bool> go{ false };
    std::vector<std::thread> threads;

    for (int w = 0; w < kWriters; ++w)
        threads.emplace_back([&go]() {
            while (!go.load()) {}
            for (int i = 0; i < kIters; ++i)
                log(LogLevel::INFO, "concurrent log line", LogOutputMode::CONSOLE);
        });
    // a thread flipping the min level concurrently
    threads.emplace_back([&go]() {
        while (!go.load()) {}
        for (int i = 0; i < kIters; ++i)
            Logger::setMinLogLevel((i & 1) ? LogLevel::FATAL : LogLevel::DEBUG);
    });

    go.store(true);
    for (auto& th : threads) th.join();

    SUCCEED(); // reaching here without a crash/sanitizer trip is the assertion
}

// 5. Independent Simulations stepped in parallel match the serial result.
//    Reproduces the neat-dnfs pattern (one Simulation per thread, no sharing).
TEST(ThreadSafety, ParallelIndependentSimulationsMatchSerial)
{
    constexpr int kSims = 16;
    constexpr int kSteps = 300;

    const std::vector<double> reference = runSerial(kSteps);
    ASSERT_FALSE(reference.empty());

    std::vector<std::vector<double>> results(kSims);
    std::vector<std::thread> threads;
    for (int s = 0; s < kSims; ++s)
        threads.emplace_back([&results, s, kSteps]() {
            auto sim = buildDetectionSim();
            sim->init();
            for (int i = 0; i < kSteps; ++i) sim->step();
            results[s] = sim->getComponent("neural field u", "activation");
        });
    for (auto& th : threads) th.join();

    for (int s = 0; s < kSims; ++s)
    {
        ASSERT_EQ(results[s].size(), reference.size()) << "sim " << s;
        for (size_t i = 0; i < reference.size(); ++i)
            EXPECT_DOUBLE_EQ(results[s][i], reference[i])
                << "sim " << s << " diverged from serial at index " << i;
    }
}
