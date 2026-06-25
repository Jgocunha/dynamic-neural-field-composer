// dnfc_suite_runner — Cross-framework algebraic equivalence test suite runner
//
// Reads every JSON file from the simulations/dnfc directory, runs the
// two-phase protocol (500 steps stimulus ON, 500 steps stimulus OFF),
// and saves the activation profiles to data/dnfc/ as CSV files.
//
// Build: add this file as an example executable in the dnfc CMakeLists.txt,
// or place it in dynamic-neural-field-composer/examples/ and rebuild.
//
// Usage: dnfc_suite_runner <simulations_dir> <output_dir>
//   simulations_dir: path to simulations/dnfc  (contains sim_NNN_*.json)
//   output_dir:      path to data/dnfc          (CSVs written here)

#include "simulation/simulation.h"
#include "simulation/simulation_file_manager.h"
#include "elements/gauss_stimulus.h"
#include "elements/neural_field.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace dnf_composer;
using namespace dnf_composer::element;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void save_csv(const std::vector<double>& data, const fs::path& path)
{
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path.string());
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) f << ',';
        f << data[i];
    }
    f << '\n';
}

// Set amplitude=0 for all stimulus elements in the simulation.
// Stimulus unique names: "gauss stimulus" (single) or "gauss stimulus N" (multi).
static void zero_all_stimuli(const std::shared_ptr<Simulation>& sim)
{
    for (auto& elem : sim->getElements()) {
        auto stim = std::dynamic_pointer_cast<GaussStimulus>(elem);
        if (!stim) continue;
        auto p = stim->getParameters();
        p.amplitude = 0.0;
        stim->setParameters(p);
    }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    const fs::path sim_dir = (argc >= 2)
        ? fs::path(argv[1])
        : fs::path(R"(C:\Users\gaspa\OneDrive - Universidade do Minho\phd-degree\journals\SoftwareX\cross-platform-validation\simulations\dnfc)");

    const fs::path out_dir = (argc >= 3)
        ? fs::path(argv[2])
        : fs::path(R"(C:\Users\gaspa\OneDrive - Universidade do Minho\phd-degree\journals\SoftwareX\cross-platform-validation\data\dnfc)");

    if (!fs::exists(sim_dir)) {
        std::cerr << "Simulations directory not found: " << sim_dir << '\n';
        return 1;
    }
    fs::create_directories(out_dir);

    // Collect and sort JSON files
    std::vector<fs::path> json_files;
    for (const auto& entry : fs::directory_iterator(sim_dir)) {
        if (entry.path().extension() == ".json")
            json_files.push_back(entry.path());
    }
    std::sort(json_files.begin(), json_files.end());

    std::cout << "Found " << json_files.size() << " JSON files in " << sim_dir << '\n';

    int ok = 0, failed = 0;

    for (const auto& json_path : json_files) {
        const std::string stem = json_path.stem().string(); // e.g. "sim_001_abssigmoid_b100"

        try {
            auto sim = std::make_shared<Simulation>(stem);
            const SimulationFileManager sfm(sim, json_path.string());
            sfm.loadElementsFromJson();

            // ── Phase 1: stimulus ON ──────────────────────────────────────
            sim->init();
            for (int t = 0; t < 500; ++t)
                sim->step();

            const auto u_with = sim->getComponent("neural field u", "activation");
            save_csv(u_with, out_dir / (stem + "_with_stimulus.csv"));

            // ── Phase 2: stimulus OFF ─────────────────────────────────────
            zero_all_stimuli(sim);
            for (int t = 0; t < 500; ++t)
                sim->step();

            const auto u_without = sim->getComponent("neural field u", "activation");
            save_csv(u_without, out_dir / (stem + "_without_stimulus.csv"));

            std::cout << "[OK]  " << stem
                      << "  peak_with=" << *std::max_element(u_with.begin(), u_with.end())
                      << "  peak_without=" << *std::max_element(u_without.begin(), u_without.end())
                      << '\n';
            ++ok;
        }
        catch (const std::exception& e) {
            std::cerr << "[ERR] " << stem << ": " << e.what() << '\n';
            ++failed;
        }
    }

    std::cout << "\nDone: " << ok << " OK, " << failed << " failed.\n";
    std::cout << "Output: " << out_dir << '\n';
    return failed > 0 ? 1 : 0;
}
