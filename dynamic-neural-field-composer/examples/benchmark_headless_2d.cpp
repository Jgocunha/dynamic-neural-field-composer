// dnfc_benchmark_headless_2d — headless 2D timing benchmark for dnfc.
//
// Programmatically creates N independent 2D neural fields (no JSON loading),
// each with 1 GaussStimulus2D + 1 GaussKernel2D (lateral) + 1 NormalNoise2D
// (amp=0.1). Times 2000 Euler steps per run (200-step warm-up discarded), 5 runs,
// and records steps/second.
//
// 2D counterpart of benchmark_headless.cpp. Same protocol and per-field
// architecture, promoted to 2D on an NxN grid given by the grid_side argument.
//
// Build inside the dnf-composer tree like the examples: register with
// add_example_executable(benchmark_headless_2d benchmark_headless_2d.cpp) in
// examples/CMakeLists.txt (links the imgui include path the logger header needs).
//
// Usage: benchmark_headless_2d [output_csv] [arch] [N_csv] [grid_side] [timed_steps] [n_runs]
//   output_csv   defaults to "timings-dnfc-2d.csv"
//   N_csv        comma-separated field counts, e.g. "10,50,100" (default "5,10,50,100")
//   grid_side    field side length, NxN grid (default 50)
//   timed_steps  timed steps per run (default 2000)
//   n_runs       runs per N (default 5)
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
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/normal_noise_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

static constexpr int    BASE_GRID    = 50;    // reference grid side the arch positions are defined on
static constexpr double TAU          = 25.0;
static constexpr double NOISE_AMP    = 0.1;    // benchmark uses A>0 so the RNG cost is measured
static constexpr int    WARMUP_STEPS = 200;
static constexpr int    TIMED_STEPS  = 2000;
static constexpr int    N_RUNS       = 5;

// ── Architecture definitions (2D) ───────────────────────────────────────────
// Reuse the representative validation sim of each band (detection 001, selection
// 021, memory 041, multi-peak 081), with the 2D amplitude adjustments from
// cross-platform-validation-2d/generate_simulations_2d.py:
//   positions on the 50-grid (pos2d = pos/2); selection kernel amp x4;
//   memory exc/inh amp x2.5 and a global inhibition of -0.05.

enum class KernelType { Gauss, MexicanHat };

struct Stim { double amp, sigma, pos; };

struct Arch {
    std::string name;
    double      h;
    KernelType  kernel;
    double      kWidth, kAmp, kGlobal;                    // Gauss
    double      kWidthExc, kAmpExc, kWidthInh, kAmpInh, kGlobalMex;  // Mexican-hat
    std::vector<Stim> stimuli;
};

static const Arch& get_arch(const std::string& name)
{
    // positions are already 2D (pos/2). amplitudes already 2D-adjusted.
    static const std::vector<Arch> archs = {
        {"detection",    -8.0,  KernelType::Gauss,      3.0, 8.0, 0.0,   0,0,0,0,0,
            {{12.0, 5.0, 25.0}}},
        {"selection",   -10.0,  KernelType::Gauss,      3.0, 20.0, -0.15, 0,0,0,0,0,  // amp 5*4
            {{10.0, 5.0, 12.5}, {10.5, 5.0, 37.5}}},
        {"memory",       -5.0,  KernelType::MexicanHat, 0,0,0,
            3.4, 44.25, 8.9, 33.75, -0.05,   // exc 17.7*2.5, inh 13.5*2.5, global -0.05
            {{15.0, 5.0, 25.0}}},
        {"multi-peak",   -8.0,  KernelType::Gauss,      2.0, 5.0, 0.0,   0,0,0,0,0,
            {{12.0, 5.0, 12.5}, {12.0, 5.0, 37.5}}},
    };
    for (const auto& a : archs)
        if (a.name == name) return a;
    std::fprintf(stderr, "Unknown arch '%s'; defaulting to detection\n", name.c_str());
    return archs[0];
}

static std::shared_ptr<Simulation> build_simulation(int N, const Arch& arch, int grid)
{
    const double pos_scale = static_cast<double>(grid) / BASE_GRID;
    auto sim = std::make_shared<Simulation>("bench2d", 25.0, 0.0, 0.0);

    for (int i = 0; i < N; ++i) {
        const std::string si = std::to_string(i);
        const ElementDimensions dims(grid, grid, 1.0, 1.0);

        // Neural field (logistic sigmoid, steepness=100)
        auto field = std::make_shared<NeuralField2D>(
            ElementCommonParameters{"field_" + si, dims},
            NeuralField2DParameters{TAU, arch.h, SigmoidFunction{0.0, 100.0}});
        sim->addElement(field);

        // Stimuli (1–3 per field)
        for (size_t s = 0; s < arch.stimuli.size(); ++s) {
            const Stim& st = arch.stimuli[s];
            // GaussStimulus2DParameters{width, amplitude, position_x, position_y, circular, normalized}
            auto stim = std::make_shared<GaussStimulus2D>(
                ElementCommonParameters{"stimulus_" + si + "_" + std::to_string(s), dims},
                GaussStimulus2DParameters{st.sigma, st.amp, st.pos * pos_scale, st.pos * pos_scale, true, false});
            sim->addElement(stim);
            field->addInput(stim);
        }

        // Lateral kernel
        if (arch.kernel == KernelType::Gauss) {
            auto kernel = std::make_shared<GaussKernel2D>(
                ElementCommonParameters{"kernel_" + si, dims},
                GaussKernel2DParameters{arch.kWidth, arch.kAmp, arch.kGlobal, true, true});
            sim->addElement(kernel);
            kernel->addInput(field);
            field->addInput(kernel);
        } else {
            auto kernel = std::make_shared<MexicanHatKernel2D>(
                ElementCommonParameters{"kernel_" + si, dims},
                MexicanHatKernel2DParameters{arch.kWidthExc, arch.kAmpExc,
                                             arch.kWidthInh, arch.kAmpInh,
                                             arch.kGlobalMex, true, true});
            sim->addElement(kernel);
            kernel->addInput(field);
            field->addInput(kernel);
        }

        // Normal noise (A>0 so the per-step RNG cost is part of the workload)
        auto noise = std::make_shared<NormalNoise2D>(
            ElementCommonParameters{"noise_" + si, dims},
            NormalNoise2DParameters{NOISE_AMP});
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
static std::vector<std::shared_ptr<GaussStimulus2D>> collect_stimuli(const std::shared_ptr<Simulation>& sim)
{
    std::vector<std::shared_ptr<GaussStimulus2D>> stimuli;
    for (const auto& el : sim->getElements())
        if (auto stim = std::dynamic_pointer_cast<GaussStimulus2D>(el))
            stimuli.push_back(stim);
    return stimuli;
}

static void establish_then_remove_stimulus(const std::vector<std::shared_ptr<GaussStimulus2D>>& stimuli,
                                            const std::vector<GaussStimulus2DParameters>& originalParams,
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

static void run_benchmark(int N, const Arch& arch, int grid, const std::string& outfile, int timedSteps, int nRuns)
{
    auto sim = build_simulation(N, arch, grid);

    std::vector<std::shared_ptr<GaussStimulus2D>> stimuli;
    std::vector<GaussStimulus2DParameters> stimuliParams;
    if (arch.name == "memory") {
        stimuli = collect_stimuli(sim);
        for (const auto& s : stimuli) stimuliParams.push_back(s->getParameters());
    }

    sim->init();

    // For "memory", establish-then-remove the stimulus before warm-up too, so the
    // discarded warm-up steps reflect the same post-establish state the timed runs
    // start from, rather than the stimulus sitting at its initial value.
    if (arch.name == "memory") establish_then_remove_stimulus(stimuli, stimuliParams, sim);

    // Warm-up
    for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

    FILE* fp = std::fopen(outfile.c_str(), "a");
    if (!fp) { std::fprintf(stderr, "Cannot open %s\n", outfile.c_str()); return; }

    for (int run = 0; run < nRuns; ++run) {
        sim->init();
        if (arch.name == "memory") establish_then_remove_stimulus(stimuli, stimuliParams, sim);

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int t = 0; t < timedSteps; ++t) sim->step();
        auto t1 = std::chrono::high_resolution_clock::now();

        double elapsed = std::chrono::duration<double>(t1 - t0).count();
        double sps     = timedSteps / elapsed;
        std::fprintf(fp,  "dnfc,default,%s,%d,headless,%d,%d,%.2f\n",
                     arch.name.c_str(), grid, N, run + 1, sps);
        std::printf("dnfc 2D %-12s fs=%dx%d N=%4d run=%d  %.1f steps/s\n",
                    arch.name.c_str(), grid, grid, N, run + 1, sps);
        std::fflush(stdout);
    }
    std::fclose(fp);
}

static void dump_final_field(const Arch& arch, int grid, int timedSteps, const std::string& dumpPath)
{
    auto sim = build_simulation(1, arch, grid);
    sim->init();
    if (arch.name == "memory") {
        auto stimuli = collect_stimuli(sim);
        std::vector<GaussStimulus2DParameters> stimuliParams;
        for (const auto& s : stimuli) stimuliParams.push_back(s->getParameters());
        establish_then_remove_stimulus(stimuli, stimuliParams, sim);
    }
    for (int t = 0; t < timedSteps; ++t) sim->step();
    auto field = std::dynamic_pointer_cast<NeuralField2D>(sim->getElement("field_0"));
    const auto& act = field->getComponent("activation");
    FILE* fp = std::fopen(dumpPath.c_str(), "w");
    if (!fp) { std::fprintf(stderr, "Cannot open %s\n", dumpPath.c_str()); return; }
    for (double v : act) std::fprintf(fp, "%.10g\n", v);
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
    // Usage: benchmark_headless_2d [output_csv] [arch] [N_csv] [grid_side] [timed_steps] [n_runs]
    //   grid_side    field side length, NxN grid (default 50)
    //   timed_steps  timed steps per run (default 5000); n_runs runs per N (default 10)
    std::string      outfile  = (argc > 1) ? argv[1] : "timings-dnfc-2d.csv";
    std::string      archName = (argc > 2) ? argv[2] : "detection";
    std::vector<int> Ns       = (argc > 3) ? parse_n_list(argv[3]) : std::vector<int>{5, 10, 50, 100};
    const int        grid       = (argc > 4) ? std::stoi(argv[4]) : BASE_GRID;
    const int        timedSteps = (argc > 5) ? std::stoi(argv[5]) : TIMED_STEPS;
    const int        nRuns      = (argc > 6) ? std::stoi(argv[6]) : N_RUNS;
    const Arch& arch = get_arch(archName);
    if (argc > 7) {
        // Behavioral-validation mode: dump field_0's final activation instead of timing.
        dump_final_field(arch, grid, timedSteps, argv[7]);
        return 0;
    }
    std::printf("dnfc 2D headless benchmark [arch=%s grid=%dx%d] -> %s\n",
                arch.name.c_str(), grid, grid, outfile.c_str());
    for (int N : Ns)
        run_benchmark(N, arch, grid, outfile, timedSteps, nRuns);
    return 0;
}
