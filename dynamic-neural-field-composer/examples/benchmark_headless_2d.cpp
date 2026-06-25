// dnfc_benchmark_headless_2d — headless 2D timing benchmark for dnfc (50x50).
//
// Programmatically creates N independent 2D neural fields (no JSON loading),
// each with 1 GaussStimulus2D + 1 GaussKernel2D (lateral) + 1 NormalNoise2D
// (amp=0). Times 5000 Euler steps and records steps/second.
//
// 2D counterpart of benchmark_headless.cpp. Same protocol and per-field
// architecture, promoted to 2D on a fixed 50x50 grid.
//
// Build inside the dnf-composer tree like the examples: register with
// add_example_executable(benchmark_headless_2d benchmark_headless_2d.cpp) in
// examples/CMakeLists.txt (links the imgui include path the logger header needs).
//
// Usage: benchmark_headless_2d [output_csv] [N_csv] [timed_steps] [n_runs]
//   output_csv   defaults to "timings-dnfc-2d.csv"
//   N_csv        comma-separated field counts, e.g. "10,50,100" (default "10,50,100,500,1000")
//   timed_steps  timed steps per run (default 5000)
//   n_runs       runs per N (default 3)
// The extra args exist for fast iteration; the no-arg defaults reproduce the
// canonical protocol.

#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/normal_noise_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static constexpr int    GRID         = 50;
static constexpr double TAU          = 25.0;
static constexpr double H            = -5.0;
static constexpr double K_WIDTH      = 3.0;
static constexpr double K_AMP        = 5.0;
static constexpr double S_WIDTH      = 5.0;
static constexpr double S_AMP        = 10.0;
static constexpr int    WARMUP_STEPS = 200;
static constexpr int    TIMED_STEPS  = 5000;
static constexpr int    N_RUNS       = 10;

static std::shared_ptr<Simulation> build_simulation(int N)
{
    auto sim = std::make_shared<Simulation>("bench2d", 25.0, 0.0, 0.0);

    // Tile N stimulus centers across the 50x50 grid (row-major), matching the
    // cosivina / cosivina-python generators.
    const int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(N))));
    const int rows = static_cast<int>(std::ceil(static_cast<double>(N) / cols));

    for (int i = 0; i < N; ++i) {
        const std::string si = std::to_string(i);
        const int c = i % cols;
        const int r = i / cols;
        const double px = (2.0 * c + 1.0) * GRID / (2.0 * cols);
        const double py = (2.0 * r + 1.0) * GRID / (2.0 * rows);

        const ElementDimensions dims(GRID, GRID, 1.0, 1.0);

        // GaussStimulus2DParameters{width, amplitude, position_x, position_y, circular, normalized}
        auto stim = std::make_shared<GaussStimulus2D>(
            ElementCommonParameters{"stimulus_" + si, dims},
            GaussStimulus2DParameters{S_WIDTH, S_AMP, px, py, true, false});

        // Neural field (logistic sigmoid, steepness=100)
        auto field = std::make_shared<NeuralField2D>(
            ElementCommonParameters{"field_" + si, dims},
            NeuralField2DParameters{TAU, H, SigmoidFunction{0.0, 100.0}});
        // Throughput benchmark: bump/stability metrics are never read here, so
        // skip the per-step flood-fill they would otherwise run.
        field->setComputeStateMetrics(false);

        // GaussKernel2DParameters{width, amplitude, amplitudeGlobal, circular, normalized}
        auto kernel = std::make_shared<GaussKernel2D>(
            ElementCommonParameters{"kernel_" + si, dims},
            GaussKernel2DParameters{K_WIDTH, K_AMP, 0.0, true, true});

        // Normal noise (amplitude=0 — present to match element count)
        auto noise = std::make_shared<NormalNoise2D>(
            ElementCommonParameters{"noise_" + si, dims},
            NormalNoise2DParameters{0.0});

        sim->addElement(stim);
        sim->addElement(field);
        sim->addElement(kernel);
        sim->addElement(noise);

        // Wire: stimulus -> field, noise -> field
        field->addInput(stim);
        field->addInput(noise);

        // Lateral: field -> kernel -> field
        kernel->addInput(field);
        field->addInput(kernel);
    }
    return sim;
}

static void run_benchmark(int N, const std::string& outfile, int timedSteps, int nRuns)
{
    auto sim = build_simulation(N);
    sim->init();

    // Warm-up
    for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

    FILE* fp = std::fopen(outfile.c_str(), "a");
    if (!fp) { std::fprintf(stderr, "Cannot open %s\n", outfile.c_str()); return; }

    for (int run = 0; run < nRuns; ++run) {
        sim->init();

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < timedSteps; ++t) sim->step();
        auto t1 = std::chrono::high_resolution_clock::now();

        double elapsed = std::chrono::duration<double>(t1 - t0).count();
        double sps     = timedSteps / elapsed;
        std::fprintf(fp,  "dnfc,headless,%d,%d,%.2f\n", N, run + 1, sps);
        std::printf("dnfc 2D headless  N=%4d  run=%d  %.1f steps/s\n", N, run + 1, sps);
        std::fflush(stdout);
    }
    std::fclose(fp);
}

static std::vector<int> parse_n_list(const std::string& s)
{
    std::vector<int> ns;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t comma = s.find(',', pos);
        const std::string tok = s.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        if (!tok.empty()) ns.push_back(std::stoi(tok));
        if (comma == std::string::npos) break;
        pos = comma + 1;
    }
    return ns;
}

int main(int argc, char* argv[])
{
    std::string outfile     = (argc > 1) ? argv[1] : "timings-dnfc-2d.csv";
    std::vector<int> Ns      = (argc > 2) ? parse_n_list(argv[2]) : std::vector<int>{10, 50, 100, 500, 1000};
    const int timedSteps     = (argc > 3) ? std::stoi(argv[3]) : TIMED_STEPS;
    const int nRuns          = (argc > 4) ? std::stoi(argv[4]) : N_RUNS;
    std::printf("dnfc 2D headless benchmark (50x50) -> %s\n", outfile.c_str());
    for (int N : Ns)
        run_benchmark(N, outfile, timedSteps, nRuns);
    return 0;
}
