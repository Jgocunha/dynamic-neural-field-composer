// dnf_composer_benchmark — in-repo throughput benchmark for 1D and 2D fields.
//
// Builds N independent neural fields (each: GaussStimulus + NeuralField +
// lateral GaussKernel + NormalNoise(amp 0)), times Euler steps, and reports
// median steps/second for N = 10/50/100 in both 1D (size 100) and 2D (50x50).
// Appends one timestamped session block to tests/benchmark/results.md so
// throughput is tracked over time as optimizations land.
//
// This mirrors examples/benchmark_headless_2d.cpp but covers both dimensions and
// writes a Markdown report. It is a manual performance run, NOT a unit test.
//
// Usage: dnf_composer_benchmark [timed_steps] [n_runs]
//   timed_steps  default 2000
//   n_runs       default 3   (median reported)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "tools/logger.h"

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

// Returns median steps/sec over nRuns of timedSteps each.
double run_cell(const std::shared_ptr<Simulation>& sim, int timedSteps, int nRuns)
{
	sim->init();
	for (int t = 0; t < WARMUP_STEPS; ++t) sim->step();

	std::vector<double> sps;
	sps.reserve(nRuns);
	for (int run = 0; run < nRuns; ++run)
	{
		sim->init();
		const auto t0 = std::chrono::high_resolution_clock::now();
		for (int t = 0; t < timedSteps; ++t) sim->step();
		const auto t1 = std::chrono::high_resolution_clock::now();
		const double elapsed = std::chrono::duration<double>(t1 - t0).count();
		sps.push_back(timedSteps / elapsed);
	}
	std::sort(sps.begin(), sps.end());
	return sps[sps.size() / 2];
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

} // namespace

int main(int argc, char* argv[])
{
	tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::FATAL);

	const int timedSteps = (argc > 1) ? std::stoi(argv[1]) : 2000;
	const int nRuns      = (argc > 2) ? std::stoi(argv[2]) : 3;
	const std::vector<int> Ns = {10, 50, 100};

	std::printf("dnf_composer benchmark  (%d steps x %d runs, median)\n", timedSteps, nRuns);

	std::vector<double> sps1d, sps2d;
	for (int N : Ns)
	{
		const double s = run_cell(build_1d(N), timedSteps, nRuns);
		sps1d.push_back(s);
		std::printf("  1D  N=%-4d  %.1f steps/s\n", N, s);
	}
	for (int N : Ns)
	{
		const double s = run_cell(build_2d(N), timedSteps, nRuns);
		sps2d.push_back(s);
		std::printf("  2D  N=%-4d  %.1f steps/s\n", N, s);
	}

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
		  << ", 2D " << GRID_2D << "x" << GRID_2D << "). One section appended per run.\n";

	f << "\n## " << timestamp()
	  << "   (dnfc " << DNF_COMPOSER_VERSION_MAJOR << "." << DNF_COMPOSER_VERSION_MINOR
	  << "." << DNF_COMPOSER_VERSION_PATCH
	  << ", " << timedSteps << " steps x " << nRuns << " runs)\n\n";
	f << "| dim | N=10 | N=50 | N=100 |\n";
	f << "|-----|-----:|-----:|------:|\n";
	f.setf(std::ios::fixed); f.precision(1);
	f << "| 1D  | " << sps1d[0] << " | " << sps1d[1] << " | " << sps1d[2] << " |\n";
	f << "| 2D  | " << sps2d[0] << " | " << sps2d[1] << " | " << sps2d[2] << " |\n";
	f << "\n_(values = median steps/sec)_\n";

	std::printf("Appended session to %s\n", path.c_str());
	return 0;
}
