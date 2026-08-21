// dnf_composer_kernelbench -- Google Benchmark microbenchmarks on the hot convolution
// and activation kernels, in isolation from a full simulation. Answers "is this specific
// rewrite faster", with proper adaptive iteration counts and repetition statistics --
// not "is the simulation faster", which is what dnf_composer_deckbench measures and
// gates on. Do not use this as a substitute for that: it measures kernels with hot
// caches and no surrounding simulation.
//
// Every kernel here is public API (tools/math.h, tools/fft_convolution.h,
// elements/activation_function.h, elements/neural_field_2d.h) -- no library
// instrumentation involved.
//
// Manual perf run, NOT a unit test -- not registered with gtest_discover_tests.
// Usage: dnf_composer_kernelbench --benchmark_repetitions=5 [other --benchmark_* flags]

#include <benchmark/benchmark.h>

#include <cmath>
#include <numeric>
#include <vector>

#include "tools/fft_convolution.h"
#include "tools/logger.h"
#include "tools/math.h"

#include "elements/activation_function.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/neural_field_2d.h"
#include "simulation/simulation.h"

using namespace dnf_composer;

namespace {

std::vector<double> makeField(int n)
{
	std::vector<double> v(n);
	for (int i = 0; i < n; ++i) v[i] = std::sin(0.13 * i);
	return v;
}

} // namespace

// ── 1D convolution -- conv_valid_into is the hot inner primitive conv_same_into's
// circular path (the one production code actually calls) shares. ──────────────────
void BM_ConvValid1D(benchmark::State& state)
{
	const int fieldSize   = static_cast<int>(state.range(0));
	const int kernelTaps  = static_cast<int>(state.range(1));
	const auto field  = makeField(fieldSize);
	const auto kernel = makeField(kernelTaps);
	std::vector<double> out(fieldSize - kernelTaps + 1);

	for (auto _ : state)
	{
		tools::math::conv_valid_into(out, field, kernel);
		benchmark::DoNotOptimize(out.data());
		benchmark::ClobberMemory();
	}
}
// {field size, kernel taps} -- 100 matches the 1D validation deck size; 200 is the
// benchmark_main.cpp/profiler_main.cpp 2D grid reused as a bigger 1D case for scale.
BENCHMARK(BM_ConvValid1D)->Args({100, 7})->Args({100, 31})->Args({200, 7})->Args({200, 31});

// ── 2D separable direct convolution (conv2d_separable_into) -- the path every 2D
// kernel element uses below kFFTTapThreshold / kFFTMinAxisSize. ─────────────────────
void BM_Conv2dSeparable(benchmark::State& state)
{
	using namespace tools::math;
	const int grid  = static_cast<int>(state.range(0));
	const double sigma = static_cast<double>(state.range(1)) / 10.0; // Args are int64_t

	const auto field = makeField(grid * grid);
	const auto kr  = computeKernelRange(sigma, 5, grid, true);
	std::vector<int> rng(static_cast<std::size_t>(kr[0]) + kr[1] + 1);
	std::iota(rng.begin(), rng.end(), -kr[0]);
	const auto taps = gaussNorm(rng, 0.0, sigma);
	const auto ext  = createExtendedIndex(grid, kr);

	Conv2dScratch<double> scratch;
	scratch.ensure(grid, grid, ext.size(), ext.size());
	std::vector<double> outBuf(grid * grid), tmpBuf(grid * grid);

	for (auto _ : state)
	{
		conv2d_separable_into(outBuf, tmpBuf, scratch, field, taps, taps, grid, grid, ext, ext);
		benchmark::DoNotOptimize(outBuf.data());
		benchmark::ClobberMemory();
	}
	state.counters["taps"] = static_cast<double>(taps.size());
}
// {grid, sigma*10} -- the 128-grid pair straddles kFFTTapThreshold exactly as
// tests/validation/data/2d_spectral/README.md's golden_001/golden_002 pair does
// (width 5.0 -> 102 taps, width 6.5 -> 134 taps), though this benchmark always takes
// the direct path since it calls conv2d_separable_into directly rather than going
// through Auto dispatch.
BENCHMARK(BM_Conv2dSeparable)
	->Args({50, 30})->Args({100, 30})->Args({128, 50})->Args({128, 65})->Args({200, 30});

// ── 2D spectral (FFTW) convolution (SpectralConvolver2D) -- the path 2D kernels take
// at/above kFFTTapThreshold and kFFTMinAxisSize. ────────────────────────────────────
void BM_Conv2dSpectral(benchmark::State& state)
{
	using namespace tools::math;
	const int grid  = static_cast<int>(state.range(0));
	const double sigma = static_cast<double>(state.range(1)) / 10.0;

	const auto field = makeField(grid * grid);
	const auto kr  = computeKernelRange(sigma, 5, grid, true);
	std::vector<int> rng(static_cast<std::size_t>(kr[0]) + kr[1] + 1);
	std::iota(rng.begin(), rng.end(), -kr[0]);
	const auto taps = gaussNorm(rng, 0.0, sigma);

	SpectralConvolver2D conv;
	conv.init(grid, grid);
	conv.setKernel(buildWrappedSeparableKernel2D(grid, grid,
		{ SeparableKernelTerm2D{ taps, kr[0], taps, kr[0], +1.0 } }));
	std::vector<double> out(grid * grid);

	for (auto _ : state)
	{
		conv.apply(field.data(), out.data());
		benchmark::DoNotOptimize(out.data());
		benchmark::ClobberMemory();
	}
	state.counters["taps"] = static_cast<double>(taps.size());
}
BENCHMARK(BM_Conv2dSpectral)->Args({128, 50})->Args({128, 65})->Args({200, 30});

// ── Sigmoid activation -- element::SigmoidFunction::apply. ─────────────────────────
void BM_SigmoidApply(benchmark::State& state)
{
	const int n = static_cast<int>(state.range(0));
	element::SigmoidFunction sf{0.0, 100.0};
	const auto field = makeField(n);
	std::vector<double> out(n);

	for (auto _ : state)
	{
		sf.apply(field, out);
		benchmark::DoNotOptimize(out.data());
		benchmark::ClobberMemory();
	}
}
// 2500 = 50x50 (decks.json "medium"), 16384 = 128x128 (decks.json "large-a"/"large-b").
BENCHMARK(BM_SigmoidApply)->Arg(2500)->Arg(16384)->Arg(40000);

// ── NeuralField2D::step -- updateInput + Euler integration + sigmoid combined, so it
// is expected to cost roughly the sum of BM_SigmoidApply plus a per-cell add/mul, not
// a new mechanism of its own; benchmarked anyway since it is the actual call site the
// simulation loop drives. A GaussStimulus2D is wired as the field's input source (the
// smallest architecture that keeps supplying non-trivial input every step) rather than
// writing components["input"] directly, which updateInput() would zero-fill again on
// the very next step since there would be no source in the cache to accumulate from.
void BM_NeuralField2DStep(benchmark::State& state)
{
	const int grid = static_cast<int>(state.range(0));
	const element::ElementDimensions dims(grid, grid, 1.0, 1.0);

	Simulation sim("kernelbench_field", 25.0, 0.0, 0.0);
	auto stim = std::make_shared<element::GaussStimulus2D>(
		element::ElementCommonParameters{"stimulus", dims},
		element::GaussStimulus2DParameters{5.0, 10.0, grid / 2.0, grid / 2.0, true, false});
	auto field = std::make_shared<element::NeuralField2D>(
		element::ElementCommonParameters{"field", dims},
		element::NeuralField2DParameters{25.0, -5.0, element::SigmoidFunction{0.0, 100.0}});
	sim.addElement(stim);
	sim.addElement(field);
	field->addInput(stim);
	sim.init();

	double t = 0.0;
	for (auto _ : state)
	{
		t += 25.0;
		field->step(t, 25.0);
		benchmark::DoNotOptimize(field->getComponentPtr("output")->data());
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_NeuralField2DStep)->Arg(50)->Arg(128);

// Not BENCHMARK_MAIN() -- BM_NeuralField2DStep constructs a Simulation per repetition,
// which logs several INFO lines per construction (element add, input wiring, init) that
// would otherwise flood every run of this tool. dnf_composer_benchmark/deckbench/
// profiler all silence the logger the same way before doing anything else.
int main(int argc, char** argv)
{
	tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::FATAL);
	benchmark::Initialize(&argc, argv);
	if (benchmark::ReportUnrecognizedArguments(argc, argv))
		return 1;
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();
	return 0;
}
