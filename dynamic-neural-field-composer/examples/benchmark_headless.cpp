// dnfc_benchmark_headless — headless timing benchmark for dnfc.
//
// Programmatically creates N independent neural fields (no JSON loading),
// each with 1 GaussStimulus + 1 GaussKernel (lateral) + 1 NormalNoise (amp=0.1).
// Times 2000 Euler steps per run (200-step warm-up discarded), 5 runs, and records
// steps/second.
//
// Usage: benchmark_headless [output_csv] [arch] [N_csv] [field_size]
//   output_csv defaults to "timings-dnfc.csv"

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static constexpr int    BASE_SIZE    = 100;   // reference grid the arch positions are defined on
static constexpr double TAU          = 25.0;
static constexpr double NOISE_AMP    = 0.1;    // benchmark uses A>0 so the RNG cost is measured
static constexpr int    WARMUP_STEPS = 200;
static constexpr int    TIMED_STEPS  = 2000;
static constexpr int    N_RUNS       = 5;

// ── Architecture definitions ────────────────────────────────────────────────
// The four benchmark architectures reuse the representative sim of each band in
// the cross-platform-validation suite (detection 001, selection 021, memory 041,
// multi-peak 081). Same params, only the field is replicated N times. This lets
// the benchmark exercise the kernel/coupling regimes real DFT models use
// (Gaussian, Mexican-hat, global inhibition, multi-stimulus) rather than a single
// trivial path.

enum class KernelType { Gauss, MexicanHat };

struct Stim { double amp, sigma, pos; };

struct Arch {
    std::string name;
    double      h;
    KernelType  kernel;
    // Gauss kernel params (kernel == Gauss)
    double      kWidth, kAmp, kGlobal;
    // Mexican-hat params (kernel == MexicanHat)
    double      kWidthExc, kAmpExc, kWidthInh, kAmpInh;
    // Stimuli (positions are relative offsets within each field's tile; see below)
    std::vector<Stim> stimuli;
};

// Representative-sim parameters (see generate_simulations.py). Stimulus positions
// here are the absolute in-field positions from the validation sims; for N>1 each
// field is tiled, so we shift them by the field's tile centre offset (keeping the
// relative geometry — e.g. selection's two stimuli 50 apart — intact).
static const Arch& get_arch(const std::string& name)
{
    static const std::vector<Arch> archs = {
        // detection 001
        {"detection",    -8.0,  KernelType::Gauss,      3.0, 8.0, 0.0,   0,0,0,0,
            {{12.0, 5.0, 50.0}}},
        // selection 021 (2 stimuli, global inhibition -0.15)
        {"selection",   -10.0,  KernelType::Gauss,      3.0, 5.0, -0.15, 0,0,0,0,
            {{10.0, 5.0, 25.0}, {10.5, 5.0, 75.0}}},
        // memory 041 (Mexican-hat, self-sustaining)
        {"memory",       -5.0,  KernelType::MexicanHat, 0,0,0,           3.4,17.7,8.9,13.5,
            {{15.0, 5.0, 50.0}}},
        // multi-peak 081 (2 narrow stimuli, narrow kernel)
        {"multi-peak",   -8.0,  KernelType::Gauss,      2.0, 5.0, 0.0,   0,0,0,0,
            {{12.0, 5.0, 25.0}, {12.0, 5.0, 75.0}}},
    };
    for (const auto& a : archs)
        if (a.name == name) return a;
    std::fprintf(stderr, "Unknown arch '%s'; defaulting to detection\n", name.c_str());
    return archs[0];
}

static std::shared_ptr<Simulation> build_simulation(int N, const Arch& arch, int field_size)
{
    const double pos_scale = static_cast<double>(field_size) / BASE_SIZE;
    auto sim = std::make_shared<Simulation>("bench", 25.0, 0.0, 0.0);

    for (int i = 0; i < N; ++i) {
        const std::string si = std::to_string(i);

        // Neural field (logistic sigmoid, steepness=100)
        auto field = std::make_shared<NeuralField>(
            ElementCommonParameters{"field_" + si,
                ElementDimensions{field_size, 1.0}},
            NeuralFieldParameters{TAU, arch.h, SigmoidFunction{0.0, 100.0}});
        sim->addElement(field);

        // Stimuli (1–3 per field depending on architecture)
        for (size_t s = 0; s < arch.stimuli.size(); ++s) {
            const Stim& st = arch.stimuli[s];
            auto stim = std::make_shared<GaussStimulus>(
                ElementCommonParameters{"stimulus_" + si + "_" + std::to_string(s),
                    ElementDimensions{field_size, 1.0}},
                GaussStimulusParameters{st.sigma, st.amp, st.pos * pos_scale, true, false});
            sim->addElement(stim);
            field->addInput(stim);
        }

        // Lateral kernel: Gauss (detection/selection/multi-peak) or
        // Mexican-hat (memory).
        if (arch.kernel == KernelType::Gauss) {
            auto kernel = std::make_shared<GaussKernel>(
                ElementCommonParameters{"kernel_" + si,
                    ElementDimensions{field_size, 1.0}},
                GaussKernelParameters{arch.kWidth, arch.kAmp, arch.kGlobal, true, true});
            sim->addElement(kernel);
            kernel->addInput(field);
            field->addInput(kernel);
        } else {
            auto kernel = std::make_shared<MexicanHatKernel>(
                ElementCommonParameters{"kernel_" + si,
                    ElementDimensions{field_size, 1.0}},
                MexicanHatKernelParameters{arch.kWidthExc, arch.kAmpExc,
                                           arch.kWidthInh, arch.kAmpInh, 0.0, true, true});
            sim->addElement(kernel);
            kernel->addInput(field);
            field->addInput(kernel);
        }

        // Normal noise (A>0 so the per-step RNG cost is part of the workload)
        auto noise = std::make_shared<NormalNoise>(
            ElementCommonParameters{"noise_" + si,
                ElementDimensions{field_size, 1.0}},
            NormalNoiseParameters{NOISE_AMP});
        sim->addElement(noise);
        field->addInput(noise);
    }
    return sim;
}

// For "memory": collect the stimuli so their amplitude can be established then
// zeroed once per run — the timed measurement covers genuine self-sustained memory
// maintenance, not stimulus-driven activity. sim->init() resets field/output state
// but not element parameters, so a stimulus zeroed by a previous run would otherwise
// stay zeroed; the cached original parameters let each run re-establish it fresh.
static std::vector<std::shared_ptr<GaussStimulus>> collect_stimuli(const std::shared_ptr<Simulation>& sim)
{
    std::vector<std::shared_ptr<GaussStimulus>> stimuli;
    for (const auto& el : sim->getElements())
        if (auto stim = std::dynamic_pointer_cast<GaussStimulus>(el))
            stimuli.push_back(stim);
    return stimuli;
}

static void establish_then_remove_stimulus(const std::vector<std::shared_ptr<GaussStimulus>>& stimuli,
                                            const std::vector<GaussStimulusParameters>& originalParams,
                                            const std::shared_ptr<Simulation>& sim)
{
    for (size_t i = 0; i < stimuli.size(); ++i) stimuli[i]->setParameters(originalParams[i]);
    for (int t = 0; t < 100; ++t) sim->step();
    for (auto& s : stimuli) {
        auto p = s->getParameters();
        p.amplitude = 0.0;
        s->setParameters(p);
    }
}

static void run_benchmark(int N, const Arch& arch, int field_size, const std::string& outfile)
{
    auto sim = build_simulation(N, arch, field_size);

    std::vector<std::shared_ptr<GaussStimulus>> stimuli;
    std::vector<GaussStimulusParameters> stimuliParams;
    if (arch.name == "memory") {
        stimuli = collect_stimuli(sim);
        for (const auto& s : stimuli) stimuliParams.push_back(s->getParameters());
    }

    sim->init();

    // For "memory", establish-then-remove the stimulus before warm-up too (matches the
    // Cedar driver), so the discarded warm-up steps reflect the same post-establish state
    // the timed runs start from, rather than the stimulus sitting at its initial value.
    if (arch.name == "memory") establish_then_remove_stimulus(stimuli, stimuliParams, sim);

    // Warm-up
    for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

    FILE* fp = std::fopen(outfile.c_str(), "a");
    if (!fp) { std::fprintf(stderr, "Cannot open %s\n", outfile.c_str()); return; }

    for (int run = 0; run < N_RUNS; ++run) {
        sim->init();
        if (arch.name == "memory") establish_then_remove_stimulus(stimuli, stimuliParams, sim);

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < TIMED_STEPS; ++t) sim->step();
        auto t1 = std::chrono::high_resolution_clock::now();

        double elapsed = std::chrono::duration<double>(t1 - t0).count();
        double sps     = TIMED_STEPS / elapsed;
        std::fprintf(fp,  "dnfc,default,%s,%d,headless,%d,%d,%.2f\n",
                     arch.name.c_str(), field_size, N, run + 1, sps);
        std::printf("dnfc %-12s fs=%4d N=%4d run=%d  %.1f steps/s\n",
                    arch.name.c_str(), field_size, N, run + 1, sps);
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
    // Usage: benchmark_headless [output_csv] [arch] [N_csv] [field_size]
    //   output_csv  default "timings-dnfc.csv"
    //   arch        detection|selection|memory|multi-peak (default detection)
    //   N_csv       comma-separated field counts (default "5,10,50,100")
    //   field_size  field length (default 100)
    std::string      outfile = (argc > 1) ? argv[1] : "timings-dnfc.csv";
    std::string      archName = (argc > 2) ? argv[2] : "detection";
    std::vector<int> Ns       = (argc > 3) ? parse_n_list(argv[3])
                                           : std::vector<int>{5, 10, 50, 100};
    const int        field_size = (argc > 4) ? std::stoi(argv[4]) : BASE_SIZE;
    const Arch& arch = get_arch(archName);
    std::printf("dnfc headless benchmark [arch=%s fs=%d] -> %s\n",
                arch.name.c_str(), field_size, outfile.c_str());
    for (int N : Ns)
        run_benchmark(N, arch, field_size, outfile);
    return 0;
}
