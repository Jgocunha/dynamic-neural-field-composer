#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <string>
#include <atomic>

#include "simulation/simulation.h"
#include "tools/utils.h"

using namespace dnf_composer;

// Regression tests for the data races reported by the neat-dnfs downstream
// TSan CI job (.claude/reports/dnf_composer-tsan-data-races.md):
//   1. Simulation::generateUniqueIdentifier() racing on std::localtime's
//      shared static buffer.
//   2. getResourceRoot()'s magic-static initialization racing with itself.
// These pass trivially on a normal build; the point is to reproduce the
// races under ThreadSanitizer.

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

    for (auto& t : threads) {
        t.join();
}
}

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

    for (auto& t : threads) {
        t.join();
}

    for (const auto& result : results) {
        EXPECT_EQ(result, results.front());
}
}
