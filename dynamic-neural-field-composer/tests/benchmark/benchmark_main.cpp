// dnf_composer_benchmark — in-repo throughput benchmark for 1D and 2D fields.
//
// Builds N independent neural fields (each: GaussStimulus + NeuralField +
// lateral GaussKernel + NormalNoise(amp 0)), times Euler steps, and reports
// median steps/second AND nanoseconds/field-cell/step for N = 10/50/100 in
// both 1D (size 100) and 2D (50x50). Appends one timestamped session block to
// tests/benchmark/results.md so throughput is tracked over time as
// optimizations land, and writes the same session as
// tests/benchmark/results/<timestamp>_<fingerprint>.json for machine-readable
// comparison (see bench_report.h / dnf_composer_deckbench --check).
//
// This mirrors examples/benchmark_headless_2d.cpp but covers both dimensions and
// writes a Markdown report. It is a manual performance run, NOT a unit test.
//
// Usage: dnf_composer_benchmark [timed_steps] [n_runs]
//   timed_steps  default 2000
//   n_runs       default 5   (median + IQR reported; 5 is the smallest count that
//                             gives a usable interquartile range, and the whole
//                             sweep still finishes in seconds)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "tools/logger.h"

#include "bench_report.h"

#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/normal_noise.h"

#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/normal_noise_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;

namespace {

// Shared field parameters (identical to the canonical headless benchmarks).
constexpr int    FIELD_SIZE_1D = 100;
constexpr int    GRID_2D       = 50;
constexpr double TAU           = 25.0;
constexpr double H             = -5.0;
constexpr double K_WIDTH       = 3.0;
constexpr double K_AMP         = 5.0;
constexpr double S_WIDTH       = 5.0;
constexpr double S_AMP         = 10.0;
constexpr int    WARMUP_STEPS  = 200;

#ifndef BENCHMARK_RESULTS_PATH
#define BENCHMARK_RESULTS_PATH "results.md"
#endif

std::shared_ptr<Simulation> build_1d(int N)
{
	auto sim = std::make_shared<Simulation>("bench1d", 25.0, 0.0, 0.0);
	for (int i = 0; i < N; ++i)
	{
		const std::string si = std::to_string(i);
		const double pos = (2.0 * i + 1.0) * FIELD_SIZE_1D / (2.0 * N);
		const ElementDimensions dims(FIELD_SIZE_1D, 1.0);

		auto stim = std::make_shared<GaussStimulus>(
			ElementCommonParameters{"stimulus_" + si, dims},
			GaussStimulusParameters{S_WIDTH, S_AMP, pos, true, false});
		auto field = std::make_shared<NeuralField>(
			ElementCommonParameters{"field_" + si, dims},
			NeuralFieldParameters{TAU, H, SigmoidFunction{0.0, 100.0}});
		auto kernel = std::make_shared<GaussKernel>(
			ElementCommonParameters{"kernel_" + si, dims},
			GaussKernelParameters{K_WIDTH, K_AMP, 0.0, true, true});
		auto noise = std::make_shared<NormalNoise>(
			ElementCommonParameters{"noise_" + si, dims},
			NormalNoiseParameters{0.0});

		sim->addElement(stim);
		sim->addElement(field);
		sim->addElement(kernel);
		sim->addElement(noise);
		field->addInput(stim);
		field->addInput(noise);
		kernel->addInput(field);
		field->addInput(kernel);
	}
	return sim;
}

std::shared_ptr<Simulation> build_2d(int N)
{
	auto sim = std::make_shared<Simulation>("bench2d", 25.0, 0.0, 0.0);
	const int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(N))));
	const int rows = static_cast<int>(std::ceil(static_cast<double>(N) / cols));
	for (int i = 0; i < N; ++i)
	{
		const std::string si = std::to_string(i);
		const int c = i % cols;
		const int r = i / cols;
		const double px = (2.0 * c + 1.0) * GRID_2D / (2.0 * cols);
		const double py = (2.0 * r + 1.0) * GRID_2D / (2.0 * rows);
		const ElementDimensions dims(GRID_2D, GRID_2D, 1.0, 1.0);

		auto stim = std::make_shared<GaussStimulus2D>(
			ElementCommonParameters{"stimulus_" + si, dims},
			GaussStimulus2DParameters{S_WIDTH, S_AMP, px, py, true, false});
		auto field = std::make_shared<NeuralField2D>(
			ElementCommonParameters{"field_" + si, dims},
			NeuralField2DParameters{TAU, H, SigmoidFunction{0.0, 100.0}});
		auto kernel = std::make_shared<GaussKernel2D>(
			ElementCommonParameters{"kernel_" + si, dims},
			GaussKernel2DParameters{K_WIDTH, K_AMP, 0.0, true, true});
		auto noise = std::make_shared<NormalNoise2D>(
			ElementCommonParameters{"noise_" + si, dims},
			NormalNoise2DParameters{0.0});

		sim->addElement(stim);
		sim->addElement(field);
		sim->addElement(kernel);
		sim->addElement(noise);
		field->addInput(stim);
		field->addInput(noise);
		kernel->addInput(field);
		field->addInput(kernel);
	}
	return sim;
}

// Returns the average wall-clock time of ONE step (seconds), one sample per run of
// timedSteps. setMeasureStepDuration(false) disables Simulation::step()'s own internal
// steady_clock::now() pair, which would otherwise add two clock calls per step on top of
// the ones this loop already makes around the whole run.
std::vector<double> timeCellSeconds(const std::shared_ptr<Simulation>& sim, int timedSteps, int nRuns)
{
	sim->init();
	sim->setMeasureStepDuration(false);
	for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

	std::vector<double> perRunStepSeconds;
	perRunStepSeconds.reserve(nRuns);
	for (int run = 0; run < nRuns; ++run)
	{
		sim->init();
		const auto t0 = std::chrono::high_resolution_clock::now();
		for (int t = 0; t < timedSteps; ++t) sim->step();
		const auto t1 = std::chrono::high_resolution_clock::now();
		const double elapsed = std::chrono::duration<double>(t1 - t0).count();
		perRunStepSeconds.push_back(elapsed / timedSteps);
	}
	return perRunStepSeconds;
}

std::string timestamp()
{
	const std::time_t now = std::time(nullptr);
	std::tm tmv{};
#if defined(_WIN32)
	localtime_s(&tmv, &now);
#else
	localtime_r(&now, &tmv);
#endif
	char buf[32];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmv);
	return buf;
}

// Filesystem-safe variant of timestamp() for the JSON sidecar's filename — ':' is not a
// valid character in a Windows path, unlike the Markdown log's "%Y-%m-%d %H:%M:%S".
std::string filenameTimestamp()
{
	const std::time_t now = std::time(nullptr);
	std::tm tmv{};
#if defined(_WIN32)
	localtime_s(&tmv, &now);
#else
	localtime_r(&now, &tmv);
#endif
	char buf[32];
	std::strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", &tmv);
	return buf;
}

} // namespace

int main(int argc, char* argv[])
{
	tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::FATAL);

	const int timedSteps = (argc > 1) ? std::stoi(argv[1]) : 2000;
	const int nRuns      = (argc > 2) ? std::stoi(argv[2]) : 5;
	const std::vector<int> Ns = {10, 50, 100};

	std::printf("dnf_composer benchmark  (%d steps x %d runs, median)\n", timedSteps, nRuns);

	std::vector<bench_report::Result> results1d, results2d;
	for (int N : Ns)
	{
		const auto samples = timeCellSeconds(build_1d(N), timedSteps, nRuns);
		const long long cells = static_cast<long long>(N) * FIELD_SIZE_1D;
		auto r = bench_report::makeResult("1d-N" + std::to_string(N), "N=" + std::to_string(N),
		                                   "synthetic-1d", "direct-1d", cells, samples);
		std::printf("  1D  N=%-4d  %.1f steps/s\n", N, r.stepsPerSec.median);
		results1d.push_back(std::move(r));
	}
	for (int N : Ns)
	{
		const auto samples = timeCellSeconds(build_2d(N), timedSteps, nRuns);
		const long long cells = static_cast<long long>(N) * GRID_2D * GRID_2D;
		auto r = bench_report::makeResult("2d-N" + std::to_string(N), "N=" + std::to_string(N),
		                                   "synthetic-2d", "direct-2d", cells, samples);
		std::printf("  2D  N=%-4d  %.1f steps/s\n", N, r.stepsPerSec.median);
		results2d.push_back(std::move(r));
	}

	// Calibration: single 1D field, same wiring as the table cells. Lets sessions
	// from different machines be compared as a ratio when their absolute numbers
	// (which depend on CPU/AVX2/build type) are not directly comparable.
	const auto calibSamples = timeCellSeconds(build_1d(1), timedSteps, nRuns);
	const auto calibResult = bench_report::makeResult("calibration-1d", "N=1", "synthetic-1d",
	                                                    "direct-1d", FIELD_SIZE_1D, calibSamples);
	const double calibration = calibResult.stepsPerSec.median;
	std::printf("  calibration (1 field, 1D)  %.1f steps/s\n", calibration);

	std::vector<bench_report::Result> allResults;
	allResults.reserve(results1d.size() + results2d.size() + 1);
	allResults.insert(allResults.end(), results1d.begin(), results1d.end());
	allResults.insert(allResults.end(), results2d.begin(), results2d.end());
	allResults.push_back(calibResult);

	bool anyNoisy = false;
	for (const auto& r : allResults)
	{
		if (r.noisy)
		{
			if (!anyNoisy)
				std::printf("\nWARNING: the following cells are too noisy to trust "
				            "(IQR/median > %.0f%%) -- re-run under scripts/bench, or "
				            "treat this session as inconclusive:\n", bench_report::kNoisyRelSpread * 100.0);
			std::printf("  %-16s IQR/median = %.1f%%\n", r.name.c_str(), r.nsPerCellStep.relSpread() * 100.0);
			anyNoisy = true;
		}
	}

	const auto env = bench_env::capture();

	// JSON sidecar: machine-readable, one file per session, alongside results.md.
	const std::filesystem::path resultsMdPath(BENCHMARK_RESULTS_PATH);
	const std::filesystem::path jsonDir = resultsMdPath.parent_path() / "results";
	std::filesystem::create_directories(jsonDir);
	const std::filesystem::path jsonPath =
		jsonDir / (filenameTimestamp() + "_" + bench_env::fingerprint(env) + ".json");
	bench_report::writeJson(jsonPath.string(), env, allResults,
	                         nlohmann::json{{"warmup_steps", WARMUP_STEPS},
	                                        {"timed_steps", timedSteps},
	                                        {"runs", nRuns}});
	std::printf("Wrote %s\n", jsonPath.string().c_str());

	// Append a dated session block to results.md (create with a title if missing).
	const std::string path = BENCHMARK_RESULTS_PATH;
	const bool existed = std::ifstream(path).good();
	std::ofstream f(path, std::ios::app);
	if (!f)
	{
		std::fprintf(stderr, "Cannot open results file: %s\n", path.c_str());
		return 1;
	}
	if (!existed)
		f << "# dnf-composer throughput benchmark\n\n"
		     "Median steps/second for N independent fields (1D size " << FIELD_SIZE_1D
		  << ", 2D " << GRID_2D << "x" << GRID_2D << "). One section appended per run.\n\n"
		     "Steps/sec is machine-dependent (CPU, AVX2 dispatch, build type all affect it) --\n"
		     "only compare sessions with matching **Env:** lines directly. The calibration\n"
		     "figure and ratio table let you roughly compare sessions across machines. Each\n"
		     "session also writes a machine-readable JSON sidecar to results/<timestamp>_\n"
		     "<fingerprint>.json (see tests/common/bench_report.h) for programmatic diffing.\n\n"
		     "Sessions before 2026-07-29 predate the **Env:** line and calibration; they all\n"
		     "ran on the reference dev machine (AMD Ryzen 5 3600, MSVC 19.44, /O2 /arch:AVX2,\n"
		     "Windows 11). Sessions before 2026-08-20 predate setMeasureStepDuration(false)\n"
		     "and the ns/cell/step / IQR columns -- absolute steps/sec numbers have a step\n"
		     "discontinuity there (two fewer steady_clock::now() calls per step).\n";

	f << "\n## " << timestamp()
	  << "   (dnfc " << DNF_COMPOSER_VERSION_MAJOR << "." << DNF_COMPOSER_VERSION_MINOR
	  << "." << DNF_COMPOSER_VERSION_PATCH
	  << ", " << timedSteps << " steps x " << nRuns << " runs)\n\n";
	f << bench_env::to_markdown(env) << "\n\n";
	f << "| dim | N=10 | N=50 | N=100 |\n";
	f << "|-----|-----:|-----:|------:|\n";
	f.setf(std::ios::fixed); f.precision(1);
	f << "| 1D  | " << results1d[0].stepsPerSec.median << " | " << results1d[1].stepsPerSec.median
	  << " | " << results1d[2].stepsPerSec.median << " |\n";
	f << "| 2D  | " << results2d[0].stepsPerSec.median << " | " << results2d[1].stepsPerSec.median
	  << " | " << results2d[2].stepsPerSec.median << " |\n";

	f << "\n**Calibration** (1 field, 1D size " << FIELD_SIZE_1D << "): " << calibration << " steps/s\n\n";
	f << "| dim | N=10 | N=50 | N=100 |\n";
	f << "|-----|-----:|-----:|------:|\n";
	f.precision(4);
	f << "| 1D  | " << results1d[0].stepsPerSec.median / calibration << " | "
	  << results1d[1].stepsPerSec.median / calibration << " | "
	  << results1d[2].stepsPerSec.median / calibration << " |\n";
	f << "| 2D  | " << results2d[0].stepsPerSec.median / calibration << " | "
	  << results2d[1].stepsPerSec.median / calibration << " | "
	  << results2d[2].stepsPerSec.median / calibration << " |\n";
	f << "\n_(values = median steps/sec; second table = ratio to calibration)_\n";

	// ns/field-cell/step: the metric that survives a change in problem size (a 1D size-100
	// field and a 2D 50x50 field are directly comparable here, unlike in steps/sec).
	f << "\n**ns/field-cell/step** (median):\n\n";
	f << "| dim | N=10 | N=50 | N=100 |\n";
	f << "|-----|-----:|-----:|------:|\n";
	f.precision(2);
	f << "| 1D  | " << results1d[0].nsPerCellStep.median << " | " << results1d[1].nsPerCellStep.median
	  << " | " << results1d[2].nsPerCellStep.median << " |\n";
	f << "| 2D  | " << results2d[0].nsPerCellStep.median << " | " << results2d[1].nsPerCellStep.median
	  << " | " << results2d[2].nsPerCellStep.median << " |\n";

	f << "\n**IQR / median** (%, on ns/field-cell/step -- above "
	  << (bench_report::kNoisyRelSpread * 100.0) << "% is flagged noisy):\n\n";
	f << "| dim | N=10 | N=50 | N=100 |\n";
	f << "|-----|-----:|-----:|------:|\n";
	f.precision(1);
	f << "| 1D  | " << results1d[0].nsPerCellStep.relSpread() * 100.0 << " | "
	  << results1d[1].nsPerCellStep.relSpread() * 100.0 << " | "
	  << results1d[2].nsPerCellStep.relSpread() * 100.0 << " |\n";
	f << "| 2D  | " << results2d[0].nsPerCellStep.relSpread() * 100.0 << " | "
	  << results2d[1].nsPerCellStep.relSpread() * 100.0 << " | "
	  << results2d[2].nsPerCellStep.relSpread() * 100.0 << " |\n";

	f << "\nJSON: `results/" << jsonPath.filename().string() << "`\n";

	std::printf("Appended session to %s\n", path.c_str());
	return 0;
}
