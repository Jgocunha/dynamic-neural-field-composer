// dnf_composer_profiler — per-step method profiler.
//
// Times how long each element's step() takes, two ways:
//   Section 1: a per-element-TYPE sweep (one of each element, minimally wired so
//              step() does real work) -> mean/median/min/max us.
//   Section 2: the representative detection sim (GaussStimulus + NeuralField +
//              lateral GaussKernel + NormalNoise) in 1D and 2D, broken down per
//              element instance with its share of the whole step.
// Appends one timestamped session block to tests/profiler/profile.md.
//
// Granularity is per element step() (no library instrumentation). Manual perf
// run, NOT a unit test.
//
// Usage: dnf_composer_profiler [iterations]   (default 20000)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "tools/logger.h"

#include "elements/neural_field.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/oscillatory_kernel.h"
#include "elements/asymmetric_gauss_kernel.h"
#include "elements/normal_noise.h"
#include "elements/correlated_normal_noise.h"
#include "elements/memory_trace.h"
#include "elements/boost_stimulus.h"
#include "elements/timed_gauss_stimulus.h"

#include "elements/neural_field_2d.h"
#include "elements/gauss_stimulus_2d.h"
#include "elements/gauss_kernel_2d.h"
#include "elements/mexican_hat_kernel_2d.h"
#include "elements/oscillatory_kernel_2d.h"
#include "elements/asymmetric_gauss_kernel_2d.h"
#include "elements/normal_noise_2d.h"
#include "elements/correlated_normal_noise_2d.h"
#include "elements/memory_trace_2d.h"
#include "elements/boost_stimulus_2d.h"
#include "elements/timed_gauss_stimulus_2d.h"

#include "elements/collapse.h"
#include "elements/expand.h"
#include "elements/resize.h"
#include "elements/resize_2d.h"

using namespace dnf_composer;
using namespace dnf_composer::element;
using clk = std::chrono::high_resolution_clock;

namespace {

constexpr int SIZE_1D = 100;
constexpr int GRID_2D = 50;

#ifndef PROFILER_RESULTS_PATH
#define PROFILER_RESULTS_PATH "profile.md"
#endif

struct Stats { double mean_us, median_us, min_us, max_us; };

// Time fn() `iters` times, return per-call stats in microseconds.
template <typename F>
Stats timeCalls(F&& fn, int iters)
{
	std::vector<double> us;
	us.reserve(iters);
	for (int i = 0; i < iters; ++i)
	{
		const auto t0 = clk::now();
		fn();
		const auto t1 = clk::now();
		us.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
	}
	std::sort(us.begin(), us.end());
	double sum = 0.0;
	for (double v : us) sum += v;
	return { sum / us.size(), us[us.size() / 2], us.front(), us.back() };
}

// ── Section 1: per-element-type sweep ──────────────────────────────────────
// Each entry builds a Simulation that feeds a source field into the element
// under test, returns {sim, elementToTime}. We init() the sim, warm up, then
// time only the element-under-test's step().

struct TypeResult { std::string name; Stats stats; bool ok; std::string note; };

ElementDimensions dims1d() { return ElementDimensions(SIZE_1D, 1.0); }
ElementDimensions dims2d() { return ElementDimensions(GRID_2D, GRID_2D, 1.0, 1.0); }

// Build a source NeuralField of the given dims with a stimulus so its output is
// non-trivial, returning the source element (already added to sim).
std::shared_ptr<Element> add1dSource(const std::shared_ptr<Simulation>& sim, const std::string& nm)
{
	auto src = std::make_shared<NeuralField>(ElementCommonParameters{nm, dims1d()},
		NeuralFieldParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}});
	auto stim = std::make_shared<GaussStimulus>(ElementCommonParameters{nm + "_s", dims1d()},
		GaussStimulusParameters{5.0, 10.0, 50.0, true, false});
	sim->addElement(src); sim->addElement(stim);
	src->addInput(stim);
	return src;
}
std::shared_ptr<Element> add2dSource(const std::shared_ptr<Simulation>& sim, const std::string& nm)
{
	auto src = std::make_shared<NeuralField2D>(ElementCommonParameters{nm, dims2d()},
		NeuralField2DParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}});
	auto stim = std::make_shared<GaussStimulus2D>(ElementCommonParameters{nm + "_s", dims2d()},
		GaussStimulus2DParameters{5.0, 10.0, 25.0, 25.0, true, false});
	sim->addElement(src); sim->addElement(stim);
	src->addInput(stim);
	return src;
}

// Run one type entry: build, init, warm, time the target's step().
TypeResult runType(const std::string& name,
                   const std::function<std::pair<std::shared_ptr<Simulation>, std::shared_ptr<Element>>()>& build,
                   int iters)
{
	try
	{
		auto [sim, target] = build();
		sim->init();
		for (int i = 0; i < 200; ++i) sim->step();
		Element* e = target.get();
		const Stats s = timeCalls([&] { e->step(0.0, 25.0); }, iters);
		return { name, s, true, "" };
	}
	catch (const std::exception& ex) { return { name, {}, false, ex.what() }; }
	catch (...)                      { return { name, {}, false, "unknown exception" }; }
}

std::vector<TypeResult> sweepTypes(int iters)
{
	std::vector<TypeResult> out;
	auto add = [&](const std::string& nm,
	               std::function<std::pair<std::shared_ptr<Simulation>, std::shared_ptr<Element>>()> b)
	{ out.push_back(runType(nm, b, iters)); };

	// Helper macros-as-lambdas: a sim with a source feeding `target`.
	auto with1dSource = [](std::function<std::shared_ptr<Element>(const ElementDimensions&)> makeTarget) {
		return [makeTarget]() {
			auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
			auto src = add1dSource(sim, "src");
			auto tgt = makeTarget(dims1d());
			sim->addElement(tgt); tgt->addInput(src);
			return std::make_pair(sim, tgt);
		};
	};
	auto with2dSource = [](std::function<std::shared_ptr<Element>(const ElementDimensions&)> makeTarget) {
		return [makeTarget]() {
			auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
			auto src = add2dSource(sim, "src");
			auto tgt = makeTarget(dims2d());
			sim->addElement(tgt); tgt->addInput(src);
			return std::make_pair(sim, tgt);
		};
	};
	auto standalone = [](std::function<std::shared_ptr<Element>()> makeTarget) {
		return [makeTarget]() {
			auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
			auto tgt = makeTarget();
			sim->addElement(tgt);
			return std::make_pair(sim, tgt);
		};
	};

	// ---- 1D ----
	add("NeuralField", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<NeuralField>(ElementCommonParameters{"t", d},
			NeuralFieldParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}}); }));
	add("GaussKernel", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<GaussKernel>(ElementCommonParameters{"t", d}, GaussKernelParameters{}); }));
	add("MexicanHatKernel", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<MexicanHatKernel>(ElementCommonParameters{"t", d}, MexicanHatKernelParameters{}); }));
	add("OscillatoryKernel", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<OscillatoryKernel>(ElementCommonParameters{"t", d}, OscillatoryKernelParameters{}); }));
	add("AsymmetricGaussKernel", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<AsymmetricGaussKernel>(ElementCommonParameters{"t", d}, AsymmetricGaussKernelParameters{}); }));
	add("NormalNoise", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<NormalNoise>(ElementCommonParameters{"t", d}, NormalNoiseParameters{0.2}); }));
	add("CorrelatedNormalNoise", standalone([] {
		return std::make_shared<CorrelatedNormalNoise>(ElementCommonParameters{"t", dims1d()}, CorrelatedNormalNoiseParameters{}); }));
	add("MemoryTrace", with1dSource([](const ElementDimensions& d) {
		return std::make_shared<MemoryTrace>(ElementCommonParameters{"t", d}, MemoryTraceParameters{}); }));
	add("GaussStimulus", standalone([] {
		return std::make_shared<GaussStimulus>(ElementCommonParameters{"t", dims1d()}, GaussStimulusParameters{}); }));
	add("TimedGaussStimulus", standalone([] {
		return std::make_shared<TimedGaussStimulus>(ElementCommonParameters{"t", dims1d()}, TimedGaussStimulusParameters{}); }));
	add("BoostStimulus", standalone([] {
		return std::make_shared<BoostStimulus>(ElementCommonParameters{"t", dims1d()}, BoostStimulusParameters{}); }));

	// ---- 2D ----
	add("NeuralField2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<NeuralField2D>(ElementCommonParameters{"t", d},
			NeuralField2DParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}}); }));
	add("GaussKernel2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<GaussKernel2D>(ElementCommonParameters{"t", d}, GaussKernel2DParameters{}); }));
	add("MexicanHatKernel2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<MexicanHatKernel2D>(ElementCommonParameters{"t", d}, MexicanHatKernel2DParameters{}); }));
	add("OscillatoryKernel2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<OscillatoryKernel2D>(ElementCommonParameters{"t", d}, OscillatoryKernel2DParameters{}); }));
	add("AsymmetricGaussKernel2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<AsymmetricGaussKernel2D>(ElementCommonParameters{"t", d}, AsymmetricGaussKernel2DParameters{}); }));
	add("NormalNoise2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<NormalNoise2D>(ElementCommonParameters{"t", d}, NormalNoise2DParameters{0.2}); }));
	add("CorrelatedNormalNoise2D", standalone([] {
		return std::make_shared<CorrelatedNormalNoise2D>(ElementCommonParameters{"t", dims2d()}, CorrelatedNormalNoise2DParameters{}); }));
	add("MemoryTrace2D", with2dSource([](const ElementDimensions& d) {
		return std::make_shared<MemoryTrace2D>(ElementCommonParameters{"t", d}, MemoryTrace2DParameters{}); }));
	add("GaussStimulus2D", standalone([] {
		return std::make_shared<GaussStimulus2D>(ElementCommonParameters{"t", dims2d()},
			GaussStimulus2DParameters{5.0, 15.0, 25.0, 25.0, true, false}); }));
	add("TimedGaussStimulus2D", standalone([] {
		return std::make_shared<TimedGaussStimulus2D>(ElementCommonParameters{"t", dims2d()}, TimedGaussStimulus2DParameters{}); }));
	add("BoostStimulus2D", standalone([] {
		return std::make_shared<BoostStimulus2D>(ElementCommonParameters{"t", dims2d()}, BoostStimulus2DParameters{}); }));

	// ---- dimension-changing (bespoke wiring) ----
	add("Collapse (2D->1D)", [] {
		auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
		auto src = add2dSource(sim, "src"); // 50x50 source
		const CollapseParameters cp{ CompressionType::SUM, ProjectionAxis::X, dims2d() };
		auto tgt = std::make_shared<Collapse>(ElementCommonParameters{"t", ElementDimensions{GRID_2D, 1.0}}, cp);
		sim->addElement(tgt); tgt->addInput(src);
		return std::make_pair(sim, std::static_pointer_cast<Element>(tgt));
	});
	add("Expand (1D->2D)", [] {
		// Expand: 1D input of size GRID_2D (the X axis) broadcast to a GRID_2D x
		// GRID_2D output. ExpandParameters carries the *input* (1D) dims; the
		// element's own dims are the 2D output.
		auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
		auto src = std::make_shared<NeuralField>(ElementCommonParameters{"src", ElementDimensions{GRID_2D, 1.0}},
			NeuralFieldParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}});
		auto stim = std::make_shared<GaussStimulus>(ElementCommonParameters{"src_s", ElementDimensions{GRID_2D, 1.0}},
			GaussStimulusParameters{5.0, 10.0, 25.0, true, false});
		src->addInput(stim);
		const ExpandParameters ep{ ProjectionAxis::X, ElementDimensions{GRID_2D, 1.0} };
		auto tgt = std::make_shared<Expand>(ElementCommonParameters{"t", dims2d()}, ep);
		sim->addElement(src); sim->addElement(stim); sim->addElement(tgt);
		tgt->addInput(src);
		return std::make_pair(sim, std::static_pointer_cast<Element>(tgt));
	});
	add("Resize (1D)", [] {
		auto sim = std::make_shared<Simulation>("p", 25.0, 0.0, 0.0);
		auto src = add1dSource(sim, "src"); // size 100
		auto tgt = std::make_shared<Resize>(ElementCommonParameters{"t", ElementDimensions{50, 1.0}}, ResizeParameters{});
		sim->addElement(tgt); tgt->addInput(src);
		return std::make_pair(sim, std::static_pointer_cast<Element>(tgt));
	});

	return out;
}

// ── Section 2: representative sim, per-instance breakdown ───────────────────
struct InstanceResult { std::string label; std::string type; double mean_us; };

std::shared_ptr<Simulation> buildDetection1d()
{
	auto sim = std::make_shared<Simulation>("det1d", 25.0, 0.0, 0.0);
	const auto d = dims1d();
	auto stim = std::make_shared<GaussStimulus>(ElementCommonParameters{"gauss stimulus", d},
		GaussStimulusParameters{5.0, 10.0, 50.0, true, false});
	auto field = std::make_shared<NeuralField>(ElementCommonParameters{"neural field u", d},
		NeuralFieldParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}});
	auto kernel = std::make_shared<GaussKernel>(ElementCommonParameters{"gauss kernel", d}, GaussKernelParameters{3.0, 5.0, 0.0, true, true});
	auto noise = std::make_shared<NormalNoise>(ElementCommonParameters{"normal noise", d}, NormalNoiseParameters{0.0});
	sim->addElement(stim); sim->addElement(field); sim->addElement(kernel); sim->addElement(noise);
	field->addInput(stim); field->addInput(noise); kernel->addInput(field); field->addInput(kernel);
	return sim;
}
std::shared_ptr<Simulation> buildDetection2d()
{
	auto sim = std::make_shared<Simulation>("det2d", 25.0, 0.0, 0.0);
	const auto d = dims2d();
	auto stim = std::make_shared<GaussStimulus2D>(ElementCommonParameters{"gauss stimulus 2d", d},
		GaussStimulus2DParameters{5.0, 10.0, 25.0, 25.0, true, false});
	auto field = std::make_shared<NeuralField2D>(ElementCommonParameters{"neural field u", d},
		NeuralField2DParameters{25.0, -5.0, SigmoidFunction{0.0, 100.0}});
	auto kernel = std::make_shared<GaussKernel2D>(ElementCommonParameters{"gauss kernel 2d", d}, GaussKernel2DParameters{3.0, 5.0, 0.0, true, true});
	auto noise = std::make_shared<NormalNoise2D>(ElementCommonParameters{"normal noise 2d", d}, NormalNoise2DParameters{0.0});
	sim->addElement(stim); sim->addElement(field); sim->addElement(kernel); sim->addElement(noise);
	field->addInput(stim); field->addInput(noise); kernel->addInput(field); field->addInput(kernel);
	return sim;
}

const char* labelName(ElementLabel l); // fwd

std::vector<InstanceResult> profileSim(const std::shared_ptr<Simulation>& sim, int iters)
{
	sim->init();
	for (int i = 0; i < 200; ++i) sim->step();

	auto elems = sim->getElements();
	std::vector<double> totals(elems.size(), 0.0);
	double t = 0.0;
	for (int it = 0; it < iters; ++it)
	{
		t += 25.0;
		for (size_t e = 0; e < elems.size(); ++e)
		{
			const auto t0 = clk::now();
			elems[e]->step(t, 25.0);
			const auto t1 = clk::now();
			totals[e] += std::chrono::duration<double, std::micro>(t1 - t0).count();
		}
	}
	std::vector<InstanceResult> out;
	for (size_t e = 0; e < elems.size(); ++e)
		out.push_back({ elems[e]->getUniqueName(), labelName(elems[e]->getLabel()), totals[e] / iters });
	return out;
}

// ── Section 3: benchmark-condition micro-profile ────────────────────────────
// Mirrors benchmark_headless_2d's detection and memory architectures at the
// real benchmark grids (noise amplitude 0.1, state metrics off, sigmoid
// steepness 100, cutoff-5 kernels). Reports per-instance step() cost plus
// direct timings of the method-level primitives inside those steps, so hot
// methods are measured, not guessed.

std::shared_ptr<Simulation> buildBench2d(int grid, bool memoryArch)
{
	auto sim = std::make_shared<Simulation>("bench2d", 25.0, 0.0, 0.0);
	const ElementDimensions d(grid, grid, 1.0, 1.0);
	const double pos = 25.0 * grid / 50.0;
	auto field = std::make_shared<NeuralField2D>(ElementCommonParameters{"field", d},
		NeuralField2DParameters{25.0, memoryArch ? -5.0 : -8.0, SigmoidFunction{0.0, 100.0}});
	auto stim = std::make_shared<GaussStimulus2D>(ElementCommonParameters{"stimulus", d},
		GaussStimulus2DParameters{memoryArch ? 15.0 : 12.0, 5.0, pos, pos, true, false});
	auto noise = std::make_shared<NormalNoise2D>(ElementCommonParameters{"noise", d},
		NormalNoise2DParameters{0.1});
	sim->addElement(field); sim->addElement(stim); sim->addElement(noise);
	field->addInput(stim); field->addInput(noise);
	if (memoryArch)
	{
		auto k = std::make_shared<MexicanHatKernel2D>(ElementCommonParameters{"kernel", d},
			MexicanHatKernel2DParameters{3.4, 44.25, 8.9, 33.75, -0.05, true, true});
		sim->addElement(k); k->addInput(field); field->addInput(k);
	}
	else
	{
		auto k = std::make_shared<GaussKernel2D>(ElementCommonParameters{"kernel", d},
			GaussKernel2DParameters{3.0, 8.0, 0.0, true, true});
		sim->addElement(k); k->addInput(field); field->addInput(k);
	}
	return sim;
}

// Ablation: field + stimulus only (no kernel, no noise) — isolates the field
// step's own cost (updateInput with one source + euler + sigmoid) from the
// full-architecture cache/plumbing effects.
std::shared_ptr<Simulation> buildFieldOnly2d(int grid)
{
	auto sim = std::make_shared<Simulation>("fieldonly2d", 25.0, 0.0, 0.0);
	const ElementDimensions d(grid, grid, 1.0, 1.0);
	auto field = std::make_shared<NeuralField2D>(ElementCommonParameters{"field", d},
		NeuralField2DParameters{25.0, -8.0, SigmoidFunction{0.0, 100.0}});
	auto stim = std::make_shared<GaussStimulus2D>(ElementCommonParameters{"stimulus", d},
		GaussStimulus2DParameters{12.0, 5.0, 25.0 * grid / 50.0, 25.0 * grid / 50.0, true, false});
	sim->addElement(field); sim->addElement(stim);
	field->addInput(stim);
	return sim;
}

struct MicroResult { std::string name; double mean_us; };

std::vector<MicroResult> microPrimitives(int grid, int iters)
{
	using namespace tools::math;
	const int n = grid * grid;
	std::vector<MicroResult> out;
	std::vector<double> a(n), b(n), c(n);
	for (int i = 0; i < n; ++i) a[i] = std::sin(0.13 * i);

	auto convCase = [&](const std::string& nm, double sigma) {
		const auto kr = computeKernelRange(sigma, 5, grid, true);
		std::vector<int> rng(static_cast<std::size_t>(kr[0]) + kr[1] + 1);
		std::iota(rng.begin(), rng.end(), -kr[0]);
		const auto taps = gaussNorm(rng, 0.0, sigma);
		const auto ext = createExtendedIndex(grid, kr);
		Conv2dScratch<double> scratch;
		scratch.ensure(grid, grid, ext.size(), ext.size());
		std::vector<double> outBuf(n), tmpBuf(n);
		const Stats s = timeCalls([&] {
			conv2d_separable_into(outBuf, tmpBuf, scratch, a, taps, taps, grid, grid, ext, ext);
		}, iters);
		out.push_back({ nm + " (" + std::to_string(taps.size()) + " taps)", s.mean_us });
	};
	convCase("conv sigma=3.0", 3.0);
	convCase("conv sigma=3.4", 3.4);
	convCase("conv sigma=8.9", 8.9);

	{
		const Stats s = timeCalls([&] {
			fillNormal(b.data(), static_cast<std::size_t>(n));
			const double scale = 0.1 / std::sqrt(25.0);
			for (int i = 0; i < n; ++i) b[i] *= scale;
		}, iters);
		out.push_back({ "noise fill+scale", s.mean_us });
	}
	{
		SigmoidFunction sf{ 0.0, 100.0 };
		const Stats s = timeCalls([&] { sf.apply(a, c); }, iters);
		out.push_back({ "sigmoid apply (mixed)", s.mean_us });
	}
	{
		// Resting-field case: u = -8 everywhere -> 1/(1+exp(+88)) ~ 6e-39, a
		// DENORMAL float result for every cell. If this is much slower than the
		// mixed case, denormal microcode assists dominate the field step.
		SigmoidFunction sf{ 0.0, 100.0 };
		std::vector<double> rest(n, -8.0);
		const Stats s = timeCalls([&] { sf.apply(rest, c); }, iters);
		out.push_back({ "sigmoid apply (resting -8)", s.mean_us });
	}
	{
		const Stats s = timeCalls([&] {
			const double* __restrict ap = a.data();
			double* __restrict cp = c.data();
			for (int i = 0; i < n; ++i) cp[i] += ap[i];
		}, iters);
		out.push_back({ "full-field add", s.mean_us });
	}
	{
		const Stats s = timeCalls([&] { std::copy(a.begin(), a.end(), c.begin()); }, iters);
		out.push_back({ "full-field copy", s.mean_us });
	}
	{
		const Stats s = timeCalls([&] {
			const double k = 1.0;
			const double* __restrict ap = a.data();
			double* __restrict cp = c.data();
			for (int i = 0; i < n; ++i) cp[i] += k * (-cp[i] + (-5.0) + ap[i]);
		}, iters);
		out.push_back({ "euler pass", s.mean_us });
	}
	return out;
}

const char* labelName(ElementLabel l)
{
	switch (l)
	{
	case ElementLabel::NEURAL_FIELD: return "NeuralField";
	case ElementLabel::NEURAL_FIELD_2D: return "NeuralField2D";
	case ElementLabel::GAUSS_KERNEL: return "GaussKernel";
	case ElementLabel::GAUSS_KERNEL_2D: return "GaussKernel2D";
	case ElementLabel::GAUSS_STIMULUS: return "GaussStimulus";
	case ElementLabel::GAUSS_STIMULUS_2D: return "GaussStimulus2D";
	case ElementLabel::NORMAL_NOISE: return "NormalNoise";
	case ElementLabel::NORMAL_NOISE_2D: return "NormalNoise2D";
	default: return "Element";
	}
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
	const int iters = (argc > 1) ? std::stoi(argv[1]) : 20000;

	std::printf("dnf_composer profiler  (%d iterations)\n", iters);

	const auto types = sweepTypes(iters);
	const auto sim1d = profileSim(buildDetection1d(), iters);
	const auto sim2d = profileSim(buildDetection2d(), iters);

	for (const auto& r : types)
		std::printf("  %-26s %s\n", r.name.c_str(),
			r.ok ? (std::to_string(r.stats.mean_us) + " us").c_str() : ("skipped: " + r.note).c_str());

	const std::string path = PROFILER_RESULTS_PATH;
	const bool existed = std::ifstream(path).good();
	std::ofstream f(path, std::ios::app);
	if (!f) { std::fprintf(stderr, "Cannot open %s\n", path.c_str()); return 1; }
	if (!existed)
		f << "# dnf-composer per-step profiler\n\n"
		     "Per-element step() timing (1D size " << SIZE_1D << ", 2D " << GRID_2D << "x" << GRID_2D
		  << "). One section appended per run.\n";

	f.setf(std::ios::fixed); f.precision(2);
	f << "\n## " << timestamp()
	  << "  (dnfc " << DNF_COMPOSER_VERSION_MAJOR << "." << DNF_COMPOSER_VERSION_MINOR
	  << "." << DNF_COMPOSER_VERSION_PATCH << ", " << iters << " iters)\n\n";

	f << "### Per element-type step()\n\n";
	f << "| element | mean us | median us | min us | max us |\n";
	f << "|---------|--------:|----------:|-------:|-------:|\n";
	for (const auto& r : types)
	{
		if (r.ok)
			f << "| " << r.name << " | " << r.stats.mean_us << " | " << r.stats.median_us
			  << " | " << r.stats.min_us << " | " << r.stats.max_us << " |\n";
		else
			f << "| " << r.name << " | _skipped_ | | | " << r.note << " |\n";
	}

	auto writeSim = [&](const char* title, const std::vector<InstanceResult>& rs) {
		double total = 0.0; for (const auto& r : rs) total += r.mean_us;
		f << "\n### " << title << "  (total " << total << " us/step)\n\n";
		f << "| element | type | mean us/step | % of step |\n";
		f << "|---------|------|-------------:|----------:|\n";
		for (const auto& r : rs)
			f << "| " << r.label << " | " << r.type << " | " << r.mean_us
			  << " | " << (total > 0 ? 100.0 * r.mean_us / total : 0.0) << "% |\n";
	};
	writeSim("Representative 1D detection sim", sim1d);
	writeSim("Representative 2D detection sim", sim2d);

	// Section 3: benchmark-condition profile (also printed to stdout).
	const int bIters = std::max(200, iters / 4);
	auto section3 = [&](int grid) {
		for (const bool mem : { false, true })
		{
			const auto rs = profileSim(buildBench2d(grid, mem), bIters);
			double total = 0.0; for (const auto& r : rs) total += r.mean_us;
			std::printf("-- bench %s@%d (total %.1f us/step) --\n",
				mem ? "memory" : "detection", grid, total);
			for (const auto& r : rs)
				std::printf("  %-12s %-18s %8.2f us  %5.1f%%\n", r.label.c_str(), r.type.c_str(),
					r.mean_us, total > 0 ? 100.0 * r.mean_us / total : 0.0);
			writeSim((std::string("Benchmark-condition ") + (mem ? "memory" : "detection")
				+ " sim @" + std::to_string(grid)).c_str(), rs);
		}
		{
			const auto rs = profileSim(buildFieldOnly2d(grid), bIters);
			std::printf("-- ablation field+stim only @%d --\n", grid);
			for (const auto& r : rs)
				std::printf("  %-12s %-18s %8.2f us\n", r.label.c_str(), r.type.c_str(), r.mean_us);
			writeSim((std::string("Ablation field+stim only @") + std::to_string(grid)).c_str(), rs);
		}
		const auto micro = microPrimitives(grid, bIters);
		std::printf("-- primitives @%d --\n", grid);
		f << "\n### Method-level primitives @" << grid << "\n\n| primitive | mean us |\n|---|--:|\n";
		for (const auto& m : micro)
		{
			std::printf("  %-28s %8.2f us\n", m.name.c_str(), m.mean_us);
			f << "| " << m.name << " | " << m.mean_us << " |\n";
		}
	};
	section3(100);
	section3(200);

	std::printf("Appended session to %s\n", path.c_str());
	return 0;
}
