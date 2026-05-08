// Benchmark: measures per-step wall-clock time of a 1D DNF simulation.
// Mirrors the Cedar dnf-benchmark for a direct apples-to-apples comparison.
//
// Architecture: GaussStimulus (size=100, amp=35, pos=50) -> NeuralField (size=100, h=-5)
// Step size: deltaT = 10 ms
// Warmup: 200 steps (excluded from stats)
// Timed:  5000 steps
//
// Reports: min / mean / max / std-dev in microseconds and throughput in steps/s.

#include "visualization/visualization.h"
#include "application/application.h"
#include "user_interface/static_layout_window.h"
#include "user_interface/main_menu_bar.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

int main()
{
    using namespace dnf_composer;
    using namespace dnf_composer::element;

    // --- Network ----------------------------------------------------------------

    // deltaT=10 (ms), tZero=0, t=0
    const auto sim = std::make_shared<Simulation>("benchmark", 10.0, 0.0, 0.0);

    // GaussStimulus: 100 nodes, width=5, amplitude=35, centre=50
    const auto gscp = ElementCommonParameters{ "gauss stimulus", 100 };
    const auto gsp  = GaussStimulusParameters{ 5.0, 35.0, 50.0, true, false };
    const auto gs   = std::make_shared<GaussStimulus>(gscp, gsp);

    // NeuralField: 100 nodes, tau=25, h=-5, sigmoid(0, 10)
    const auto nfcp = ElementCommonParameters{ "neural field", 100 };
    const auto nfp  = NeuralFieldParameters{ 25.0, -5.0, SigmoidFunction{ 0.0, 10.0 } };
    const auto nf   = std::make_shared<NeuralField>(nfcp, nfp);

    sim->addElement(gs);
    sim->addElement(nf);
    nf->addInput(gs);

    nf->setComputeStateMetrics(false);
    sim->setMeasureStepDuration(false);

    sim->init();

    // --- Warmup -----------------------------------------------------------------

    constexpr unsigned int N_WARMUP = 200;
    constexpr unsigned int N_BENCH  = 5000;

    for (unsigned int i = 0; i < N_WARMUP; ++i)
        sim->step();

    // --- Benchmark --------------------------------------------------------------

    using Clock = std::chrono::high_resolution_clock;
    using us    = std::chrono::duration<double, std::micro>;

    std::vector<double> samples(N_BENCH);

    for (unsigned int i = 0; i < N_BENCH; ++i)
    {
        const auto t0 = Clock::now();
        sim->step();
        const auto t1 = Clock::now();
        samples[i] = std::chrono::duration_cast<us>(t1 - t0).count();
    }

    // --- Statistics -------------------------------------------------------------

    double sum  = 0.0;
    double vmin = std::numeric_limits<double>::max();
    double vmax = 0.0;

    for (const double s : samples)
    {
        sum += s;
        if (s < vmin) vmin = s;
        if (s > vmax) vmax = s;
    }

    const double mean = sum / N_BENCH;

    double var = 0.0;
    for (const double s : samples)
    {
        const double d = s - mean;
        var += d * d;
    }
    const double stddev = std::sqrt(var / N_BENCH);

    // Built-in timing cross-check (last step duration reported by Simulation)
    const double builtin_us =
        std::chrono::duration_cast<us>(sim->getLastStepDuration()).count();

    // --- Report -----------------------------------------------------------------

    std::cout << "\nDNF Composer step benchmark  (field size = 100, dt = 10 ms)\n"
              << "  warmup steps : " << N_WARMUP << "\n"
              << "  bench  steps : " << N_BENCH  << "\n\n"
              << std::fixed << std::setprecision(2)
              << "  min    : " << std::setw(9) << vmin      << " us\n"
              << "  mean   : " << std::setw(9) << mean      << " us\n"
              << "  max    : " << std::setw(9) << vmax      << " us\n"
              << "  stddev : " << std::setw(9) << stddev    << " us\n"
              << "  last   : " << std::setw(9) << builtin_us << " us  (sim->getLastStepDuration())\n"
              << "\n  throughput : "
              << std::setprecision(0) << (1e6 / mean) << " steps/s\n"
              << std::endl;

    return 0;
}
