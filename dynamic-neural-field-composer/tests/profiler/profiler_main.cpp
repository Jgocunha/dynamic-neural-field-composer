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

	std::printf("Appended session to %s\n", path.c_str());
	return 0;
}
