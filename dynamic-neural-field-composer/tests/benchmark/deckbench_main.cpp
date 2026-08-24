// dnf_composer_deckbench — reproducible performance baseline over a FIXED set of committed
// simulation decks (tests/benchmark/decks.json), rather than synthetic N-fields
// architectures. Every deck is also a correctness fixture with a committed reference CSV
// (see tests/benchmark/DECKS.md), so a deck that regresses in speed can be handed straight
// to the matching validation suite to confirm its numerics did not also move.
//
// This is a manual performance run, NOT a unit test -- not registered with
// gtest_discover_tests, same as dnf_composer_benchmark and dnf_composer_profiler.
//
// Usage: dnf_composer_deckbench [--decks <manifest.json>] [--steps N] [--runs N]
//                                [--json <out.json>] [--paths]
//                                [--record [--force] | --check [--threshold PCT]]
//   --decks   deck manifest, default: the one baked in at configure time
//   --steps   timed steps per run, default 2000
//   --runs    runs per deck, default 5 (median + IQR reported)
//   --json    output path, default tests/benchmark/results/deckbench_<timestamp>_<fp>.json
//   --paths   for every "large*" tier deck, ALSO time under ForceDirect and ForceSpectral
//             (tools::math::ScopedConvolutionMode) so Auto's dispatch choice is confirmed
//             empirically instead of assumed. There is no other way to observe which
//             convolution path Auto took without instrumenting the library. Ignored (a
//             warning is printed) together with --record/--check, which always compare
//             the plain Auto-mode measurement so every deck has exactly one entry.
//
// --record / --check compare against a per-machine baseline at
// tests/benchmark/baselines/<fingerprint>.json -- see the exit-code table on
// runCheck() below. NEVER run these from CI: GitHub-hosted runners are shared VMs
// whose run-to-run spread exceeds any regression worth gating on (see
// .claude/performance-workplan/WP-07-machine-hygiene-scripts.md). This is a local,
// pre-PR check, ideally run under scripts/bench.ps1 / scripts/bench.sh.

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "simulation/simulation.h"
#include "simulation/simulation_file_manager.h"
#include "tools/fft_convolution.h"
#include "tools/logger.h"

#include "bench_report.h"

using namespace dnf_composer;

namespace {

constexpr int WARMUP_STEPS = 200;

#ifndef DECKBENCH_DECKS_MANIFEST
#define DECKBENCH_DECKS_MANIFEST "decks.json"
#endif
#ifndef DECKBENCH_VALIDATION_DATA_DIR
#define DECKBENCH_VALIDATION_DATA_DIR "."
#endif
#ifndef DECKBENCH_RESULTS_DIR
#define DECKBENCH_RESULTS_DIR "results"
#endif
#ifndef DECKBENCH_BASELINES_DIR
#define DECKBENCH_BASELINES_DIR "baselines"
#endif

// Measured, not guessed: .claude/notes/perf-noise-floor.md records a 10-session
// noise-floor measurement under scripts/bench (median IQR/median 1.89%, worst session
// 3.79%) on the reference dev machine. 5% sits comfortably above both, with real margin
// rather than being tuned to rarely fire. Re-measure and revise here (and in
// tests/benchmark/DECKS.md) if that changes materially on a different reference machine.
constexpr double kDefaultThresholdPct = 5.0;

struct DeckSpec
{
	std::string tier;
	std::string path;
	std::string architecture;
	std::string expectPath;
	long long   fieldCells = 0;
};

std::vector<DeckSpec> loadManifest(const std::filesystem::path& manifestPath)
{
	std::ifstream f(manifestPath);
	if (!f)
		throw std::runtime_error("Cannot open deck manifest: " + manifestPath.string());
	nlohmann::json j;
	f >> j;

	std::vector<DeckSpec> decks;
	for (const auto& d : j.at("decks"))
	{
		DeckSpec spec;
		spec.tier         = d.at("tier").get<std::string>();
		spec.path         = d.at("path").get<std::string>();
		spec.architecture = d.at("architecture").get<std::string>();
		spec.expectPath   = d.at("expect_path").get<std::string>();
		spec.fieldCells   = d.at("field_cells").get<long long>();
		decks.push_back(std::move(spec));
	}
	return decks;
}

// FNV-1a64 over the deck file's raw bytes -- a change detector, not a cryptographic
// digest. Reuses bench_env's hash/hex helpers so this doesn't grow a second
// implementation of the same 15 lines.
std::string hashDeckFile(const std::filesystem::path& path)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
		throw std::runtime_error("Cannot open deck for hashing: " + path.string());
	std::ostringstream contents;
	contents << f.rdbuf();
	return bench_env::detail::to_hex16(bench_env::detail::fnv1a64(contents.str()));
}

std::shared_ptr<Simulation> loadDeck(const std::filesystem::path& jsonPath)
{
	auto sim = std::make_shared<Simulation>(jsonPath.stem().string());
	const SimulationFileManager sfm(sim, jsonPath.string());
	sfm.loadElementsFromJson();
	return sim;
}

// Average wall-clock time of ONE step (seconds), one sample per run of timedSteps.
// setMeasureStepDuration(false) removes Simulation::step()'s own internal
// steady_clock::now() pair from the timed loop. Loads the deck fresh so warmup and every
// per-run sim->init() are covered by whatever ConvolutionMode override (if any) the
// caller has active -- 2D kernel elements decide their direct-vs-spectral dispatch inside
// init(), not per step, so the override must be active across every init() in this call,
// not just around the timed loop.
std::vector<double> timeDeckSeconds(const std::filesystem::path& jsonPath, int timedSteps, int nRuns)
{
	auto sim = loadDeck(jsonPath);
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

void printResultLine(const bench_report::Result& r)
{
	std::printf("  %-22s %10.1f steps/s   %8.2f ns/cell/step  (IQR %.1f%%)%s\n",
	            r.name.c_str(), r.stepsPerSec.median, r.nsPerCellStep.median,
	            r.nsPerCellStep.relSpread() * 100.0, r.noisy ? "  [NOISY]" : "");
}

std::filesystem::path baselinePathFor(const bench_env::Env& env)
{
	return std::filesystem::path(DECKBENCH_BASELINES_DIR) / (bench_env::fingerprint(env) + ".json");
}

// --record: write tests/benchmark/baselines/<fingerprint>.json from `results`. Refuses to
// overwrite an existing baseline without --force, and refuses outright if any result is
// noisy -- a baseline recorded on a noisy machine poisons every later comparison.
int runRecord(const bench_env::Env& env, const std::vector<bench_report::Result>& results,
              const nlohmann::json& config, bool force)
{
	for (const auto& r : results)
	{
		if (r.noisy)
		{
			std::fprintf(stderr,
				"--record refused: %s is too noisy to trust (IQR/median = %.1f%%).\n"
				"Re-run under scripts/bench.ps1 / scripts/bench.sh -- a baseline recorded\n"
				"here would poison every later --check comparison.\n",
				r.name.c_str(), r.nsPerCellStep.relSpread() * 100.0);
			return 1;
		}
	}

	const std::filesystem::path baselinesDir(DECKBENCH_BASELINES_DIR);
	std::filesystem::create_directories(baselinesDir);
	const auto path = baselinePathFor(env);

	if (std::filesystem::exists(path) && !force)
	{
		std::fprintf(stderr,
			"--record refused: %s already exists. Pass --force to overwrite deliberately.\n",
			path.string().c_str());
		return 1;
	}

	bench_report::writeJson(path.string(), env, results, config);
	std::printf("Recorded baseline: %s\n", path.string().c_str());
	return 0;
}

struct BaselineEntry { double nsPerCellStepMedian = 0.0; bool noisy = false; std::string deckHash; };

// --check: compare `results` against tests/benchmark/baselines/<fingerprint>.json.
//
// Exit codes:
//   0  every deck within threshold
//   1  at least one deck regressed past threshold
//   2  no baseline for this fingerprint, or a deck's hash no longer matches the
//      baseline's -- refuse to compare rather than silently comparing apples to oranges
//   3  a current or baseline result is too noisy to conclude anything -- inconclusive,
//      NOT a failure (a gate that fails on noise gets ignored within a week)
int runCheck(const bench_env::Env& env, const std::vector<bench_report::Result>& results,
             double thresholdPct)
{
	const auto path = baselinePathFor(env);
	if (!std::filesystem::exists(path))
	{
		std::fprintf(stderr,
			"--check refused: no baseline for this environment's fingerprint (%s) at %s.\n"
			"Run --record on a known-good tree first.\n",
			bench_env::fingerprint(env).c_str(), path.string().c_str());
		return 2;
	}

	// A truncated or hand-edited baseline must refuse the comparison (exit 2), not abort
	// on an uncaught parse exception -- the caller cannot tell a crash from a real
	// regression, and this file is one people are told to inspect and commit.
	nlohmann::json baselineJson;
	try
	{
		std::ifstream f(path);
		f >> baselineJson;
	}
	catch (const std::exception& e)
	{
		std::fprintf(stderr,
			"--check refused: %s is not readable as JSON (%s).\n"
			"Re-record the baseline on a known-good tree.\n",
			path.string().c_str(), e.what());
		return 2;
	}

	if (baselineJson.value("fingerprint", std::string{}) != bench_env::fingerprint(env))
	{
		std::fprintf(stderr,
			"--check refused: %s's own fingerprint field does not match this environment's.\n"
			"This baseline file appears to have been copied or edited -- re-record it.\n",
			path.string().c_str());
		return 2;
	}

	// Parsed above, but the shape is still unverified: at() throws on a baseline that is
	// valid JSON yet missing the keys this reads. Same reasoning as the parse guard --
	// refuse the comparison rather than abort.
	std::unordered_map<std::string, BaselineEntry> baseline;
	try
	{
		for (const auto& r : baselineJson.at("results"))
		{
			BaselineEntry e;
			e.nsPerCellStepMedian = r.at("ns_per_cell_step").at("median").get<double>();
			e.noisy               = r.value("noisy", false);
			e.deckHash            = r.value("deck_hash", std::string{});
			baseline[r.at("name").get<std::string>()] = e;
		}
	}
	catch (const std::exception& e)
	{
		std::fprintf(stderr,
			"--check refused: %s does not have the expected shape (%s).\n"
			"Re-record the baseline on a known-good tree.\n",
			path.string().c_str(), e.what());
		return 2;
	}

	std::printf("\n%-22s %14s %14s %10s  %s\n", "deck", "baseline", "current", "delta", "verdict");
	std::printf("%-22s %14s %14s %10s  %s\n", "----", "--------", "-------", "-----", "-------");

	bool anyMismatch  = false;
	bool anyNoisy     = false;
	bool anyRegressed = false;

	for (const auto& r : results)
	{
		const auto it = baseline.find(r.name);
		if (it == baseline.end())
		{
			std::printf("%-22s %14s %14s %10s  NOT IN BASELINE\n", r.name.c_str(), "--", "--", "--");
			anyMismatch = true;
			continue;
		}
		const auto& b = it->second;
		if (!b.deckHash.empty() && !r.deckHash.empty() && b.deckHash != r.deckHash)
		{
			std::printf("%-22s %14s %14s %10s  DECK CHANGED SINCE BASELINE\n",
			            r.name.c_str(), "--", "--", "--");
			anyMismatch = true;
			continue;
		}

		const double deltaPct = b.nsPerCellStepMedian > 0.0
			? 100.0 * (r.nsPerCellStep.median - b.nsPerCellStepMedian) / b.nsPerCellStepMedian
			: 0.0;
		const bool noisyHere = r.noisy || b.noisy;
		const bool regressed = deltaPct > thresholdPct;
		anyNoisy     = anyNoisy || noisyHere;
		anyRegressed = anyRegressed || regressed;

		const char* verdict = noisyHere ? "NOISY" : (regressed ? "REGRESSED" : "ok");
		std::printf("%-22s %14.2f %14.2f %+9.2f%%  %s\n",
		            r.name.c_str(), b.nsPerCellStepMedian, r.nsPerCellStep.median, deltaPct, verdict);
	}
	std::printf("(ns/field-cell/step, median; delta = current vs baseline; threshold %.1f%%)\n",
	            thresholdPct);

	if (anyMismatch)
	{
		std::fprintf(stderr,
			"\n--check refused: see NOT IN BASELINE / DECK CHANGED rows above. A deck that\n"
			"changed (edited, added, or removed) invalidates the comparison regardless of\n"
			"the timings -- re-record the baseline once the change is deliberate.\n");
		return 2;
	}
	if (anyNoisy)
	{
		std::fprintf(stderr,
			"\n--check inconclusive: see NOISY rows above. A noisy run never fails this\n"
			"gate -- re-run under scripts/bench.ps1 / scripts/bench.sh and check again.\n");
		return 3;
	}
	if (anyRegressed)
	{
		std::fprintf(stderr, "\n--check FAILED: see REGRESSED rows above.\n");
		return 1;
	}
	std::printf("\n--check passed: every deck within %.1f%%.\n", thresholdPct);
	return 0;
}

} // namespace

int main(int argc, char* argv[])
{
	tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::FATAL);

	std::string manifestArg = DECKBENCH_DECKS_MANIFEST;
	int         timedSteps  = 2000;
	int         nRuns       = 5;
	std::string jsonArg;
	bool        pathsMode  = false;
	bool        recordMode = false;
	bool        checkMode  = false;
	bool        force      = false;
	double      thresholdPct = kDefaultThresholdPct;

	// std::stoi/stod throw on text that is not a number, and a zero or negative step or
	// run count produces a meaningless measurement rather than an error. Reject both here
	// so the tool prints a usage message instead of terminating on an uncaught exception
	// part-way through a timing session.
	const auto parsePositiveInt = [](const char* text, const char* flag, int& out) {
		try
		{
			const int value = std::stoi(text);
			if (value <= 0)
			{
				std::fprintf(stderr, "%s must be a positive integer, got '%s'.\n", flag, text);
				return false;
			}
			out = value;
			return true;
		}
		catch (const std::exception&)
		{
			std::fprintf(stderr, "%s expects an integer, got '%s'.\n", flag, text);
			return false;
		}
	};

	for (int i = 1; i < argc; ++i)
	{
		const std::string a = argv[i];
		if (a == "--decks" && i + 1 < argc)         manifestArg  = argv[++i];
		else if (a == "--steps" && i + 1 < argc)
		{
			if (!parsePositiveInt(argv[++i], "--steps", timedSteps)) return 1;
		}
		else if (a == "--runs" && i + 1 < argc)
		{
			if (!parsePositiveInt(argv[++i], "--runs", nRuns)) return 1;
		}
		else if (a == "--json" && i + 1 < argc)     jsonArg      = argv[++i];
		else if (a == "--paths")                    pathsMode    = true;
		else if (a == "--record")                   recordMode   = true;
		else if (a == "--check")                    checkMode    = true;
		else if (a == "--force")                    force        = true;
		else if (a == "--threshold" && i + 1 < argc)
		{
			const char* text = argv[++i];
			try
			{
				thresholdPct = std::stod(text);
			}
			catch (const std::exception&)
			{
				std::fprintf(stderr, "--threshold expects a number, got '%s'.\n", text);
				return 1;
			}
			if (thresholdPct <= 0.0)
			{
				std::fprintf(stderr, "--threshold must be positive, got '%s'.\n", text);
				return 1;
			}
		}
		else
		{
			std::fprintf(stderr, "Unknown argument: %s\n", a.c_str());
			return 1;
		}
	}

	if (recordMode && checkMode)
	{
		std::fprintf(stderr, "--record and --check are mutually exclusive.\n");
		return 1;
	}
	if ((recordMode || checkMode) && pathsMode)
	{
		std::fprintf(stderr,
			"Note: --paths is ignored with --record/--check, which always compare the plain\n"
			"Auto-mode measurement so every deck has exactly one baseline entry.\n");
		pathsMode = false;
	}

	std::vector<DeckSpec> decks;
	try
	{
		decks = loadManifest(manifestArg);
	}
	catch (const std::exception& e)
	{
		std::fprintf(stderr, "Failed to load deck manifest '%s': %s\n", manifestArg.c_str(), e.what());
		return 1;
	}

	const std::filesystem::path dataRoot(DECKBENCH_VALIDATION_DATA_DIR);
	std::printf("dnf_composer deckbench  (%d steps x %d runs, median)%s\n",
	            timedSteps, nRuns, pathsMode ? "  [--paths]" : "");

	std::vector<bench_report::Result> results;
	bool anyFailure = false;

	for (const auto& spec : decks)
	{
		const std::filesystem::path fullPath = dataRoot / spec.path;
		if (!std::filesystem::exists(fullPath))
		{
			std::fprintf(stderr, "Deck not found: %s\n", fullPath.string().c_str());
			anyFailure = true;
			continue;
		}

		std::string hash;
		try
		{
			hash = hashDeckFile(fullPath);
		}
		catch (const std::exception& e)
		{
			std::fprintf(stderr, "Failed to hash deck %s: %s\n", spec.path.c_str(), e.what());
			anyFailure = true;
			continue;
		}

		std::printf("\n%s (%s):\n", spec.tier.c_str(), spec.path.c_str());

		const bool isLargeTier = spec.tier.rfind("large", 0) == 0;

		try
		{
			if (pathsMode && isLargeTier)
			{
				const auto autoSamples = timeDeckSeconds(fullPath, timedSteps, nRuns);
				auto autoResult = bench_report::makeResult(spec.tier + "-auto", spec.tier,
				                                            spec.architecture, "auto",
				                                            spec.fieldCells, autoSamples);
				autoResult.deckHash = hash;

				std::vector<double> directSamples;
				{
					const tools::math::ScopedConvolutionMode guard(tools::math::ConvolutionMode::ForceDirect);
					directSamples = timeDeckSeconds(fullPath, timedSteps, nRuns);
				}
				auto directResult = bench_report::makeResult(spec.tier + "-forcedirect", spec.tier,
				                                              spec.architecture, "direct-2d",
				                                              spec.fieldCells, directSamples);
				directResult.deckHash = hash;

				std::vector<double> spectralSamples;
				{
					const tools::math::ScopedConvolutionMode guard(tools::math::ConvolutionMode::ForceSpectral);
					spectralSamples = timeDeckSeconds(fullPath, timedSteps, nRuns);
				}
				auto spectralResult = bench_report::makeResult(spec.tier + "-forcespectral", spec.tier,
				                                                spec.architecture, "spectral-2d",
				                                                spec.fieldCells, spectralSamples);
				spectralResult.deckHash = hash;

				// Empirical dispatch confirmation: Auto's median should sit close to
				// WHICHEVER of the forced paths it actually took, not halfway between.
				// This is the only way to observe the dispatch choice without
				// instrumenting the library -- see the file header.
				const double toDirect   = std::abs(autoResult.nsPerCellStep.median - directResult.nsPerCellStep.median);
				const double toSpectral = std::abs(autoResult.nsPerCellStep.median - spectralResult.nsPerCellStep.median);
				const std::string observedPath = (toDirect < toSpectral) ? "direct-2d" : "spectral-2d";
				autoResult.path = observedPath;

				printResultLine(directResult);
				printResultLine(spectralResult);
				printResultLine(autoResult);
				std::printf("    Auto observed: %s (expected: %s)%s\n",
				            observedPath.c_str(), spec.expectPath.c_str(),
				            observedPath == spec.expectPath ? "" : "  *** MISMATCH ***");

				if (observedPath != spec.expectPath)
					std::fprintf(stderr,
						"\nWARNING: %s -- Auto dispatched to %s but DECKS.md documents %s.\n"
						"This means either tools::math::shouldUseSpectral2D's dispatch rule or\n"
						"tests/validation/data/2d_spectral/README.md is now wrong -- that finding\n"
						"outranks this benchmark session. Investigate before trusting this run.\n",
						spec.tier.c_str(), observedPath.c_str(), spec.expectPath.c_str());

				results.push_back(std::move(directResult));
				results.push_back(std::move(spectralResult));
				results.push_back(std::move(autoResult));
			}
			else
			{
				const auto samples = timeDeckSeconds(fullPath, timedSteps, nRuns);
				auto result = bench_report::makeResult(spec.tier, spec.tier, spec.architecture,
				                                        spec.expectPath, spec.fieldCells, samples);
				result.deckHash = hash;
				printResultLine(result);
				results.push_back(std::move(result));
			}
		}
		catch (const std::exception& e)
		{
			std::fprintf(stderr, "Failed to time deck %s: %s\n", spec.path.c_str(), e.what());
			anyFailure = true;
		}
	}

	if (anyFailure)
	{
		std::fprintf(stderr, "\nOne or more decks failed to load or time -- aborting before "
		                      "writing a JSON report.\n");
		return 1;
	}

	bool anyNoisy = false;
	for (const auto& r : results)
		anyNoisy = anyNoisy || r.noisy;
	if (anyNoisy)
		std::printf("\nWARNING: at least one result is too noisy to trust "
		            "(IQR/median > %.0f%%) -- re-run under scripts/bench.\n",
		            bench_report::kNoisyRelSpread * 100.0);

	const auto env = bench_env::capture();
	const auto config = nlohmann::json{{"warmup_steps", WARMUP_STEPS},
	                                    {"timed_steps", timedSteps},
	                                    {"runs", nRuns},
	                                    {"paths_mode", pathsMode},
	                                    {"manifest", manifestArg}};

	if (recordMode)
		return runRecord(env, results, config, force);
	if (checkMode)
		return runCheck(env, results, thresholdPct);

	std::filesystem::path jsonPath;
	if (!jsonArg.empty())
	{
		jsonPath = jsonArg;
	}
	else
	{
		const std::filesystem::path resultsDir(DECKBENCH_RESULTS_DIR);
		std::filesystem::create_directories(resultsDir);
		jsonPath = resultsDir / ("deckbench_" + filenameTimestamp() + "_" + bench_env::fingerprint(env) + ".json");
	}

	bench_report::writeJson(jsonPath.string(), env, results, config);
	std::printf("\nWrote %s\n", jsonPath.string().c_str());

	return 0;
}
