// dnfc_benchmark_headless — headless timing benchmark for dnfc.
//
// Programmatically creates N independent neural fields (no JSON loading),
// each with 1 GaussStimulus + 1 GaussKernel (lateral) + 1 NormalNoise (amp=0).
// Times 5000 Euler steps and records steps/second.
//
// Usage: dnfc_benchmark_headless [output_csv]
//   output_csv defaults to "timings.csv"

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/normal_noise.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static constexpr int    FIELD_SIZE   = 100;
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
    auto sim = std::make_shared<Simulation>("bench", 25.0, 0.0, 0.0);

    for (int i = 0; i < N; ++i) {
        const std::string si = std::to_string(i);
        double pos = (2.0 * i + 1.0) * FIELD_SIZE / (2.0 * N);

        // Gauss stimulus
        auto stim = std::make_shared<GaussStimulus>(
            ElementCommonParameters{"stimulus_" + si,
                ElementDimensions{FIELD_SIZE}},
            GaussStimulusParameters{S_WIDTH, S_AMP, pos, true, false});

        // Neural field (logistic sigmoid, steepness=100)
        auto field = std::make_shared<NeuralField>(
            ElementCommonParameters{"field_" + si,
                ElementDimensions{FIELD_SIZE}},
            NeuralFieldParameters{TAU, H, SigmoidFunction{0.0, 100.0}});

        // Gauss kernel (lateral self-connection)
        auto kernel = std::make_shared<GaussKernel>(
            ElementCommonParameters{"kernel_" + si,
                ElementDimensions{FIELD_SIZE}},
            GaussKernelParameters{K_WIDTH, K_AMP, 0.0, true, true});

        // Normal noise (amplitude=0 — present to match element count)
        auto noise = std::make_shared<NormalNoise>(
            ElementCommonParameters{"noise_" + si,
                ElementDimensions{FIELD_SIZE}},
            NormalNoiseParameters{0.0});

        sim->addElement(stim);
        sim->addElement(field);
        sim->addElement(kernel);
        sim->addElement(noise);

        // Wire: stimulus → field, noise → field
        field->addInput(stim);
        field->addInput(noise);

        // Lateral: field → kernel → field
        kernel->addInput(field);
        field->addInput(kernel);
    }
    return sim;
}

static void run_benchmark(int N, const std::string& outfile)
{
    auto sim = build_simulation(N);
    sim->init();

    // Warm-up
    for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

    FILE* fp = std::fopen(outfile.c_str(), "a");
    if (!fp) { std::fprintf(stderr, "Cannot open %s\n", outfile.c_str()); return; }

    for (int run = 0; run < N_RUNS; ++run) {
        sim->init();

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < TIMED_STEPS; ++t) sim->step();
        auto t1 = std::chrono::high_resolution_clock::now();

        double elapsed = std::chrono::duration<double>(t1 - t0).count();
        double sps     = TIMED_STEPS / elapsed;
        std::fprintf(fp,  "dnfc,headless,%d,%d,%.2f\n", N, run + 1, sps);
        std::printf("dnfc headless  N=%3d  run=%d  %.1f steps/s\n", N, run + 1, sps);
    }
    std::fclose(fp);
}

int main(int argc, char* argv[])
{
    std::string outfile = (argc > 1) ? argv[1] : "timings-dnfc.csv";
    std::printf("dnfc headless benchmark → %s\n", outfile.c_str());
    for (int N : {10, 50, 100, 500, 1000})
        run_benchmark(N, outfile);
    return 0;
}
