#pragma once

// Shared statistics, normalized metric and JSON writer for the manual perf tools
// (benchmark_main.cpp, deckbench_main.cpp, profiler_main.cpp). Written once so the three
// tools cannot drift into reporting the same kind of number three different ways.
//
// Wall-clock throughput (steps/sec) does not survive a change in problem size, so every
// result also carries nanoseconds-per-field-cell-per-step (nsPerCellPerStep) — comparable
// across grid sizes within the same architecture family. Every stats block reports the
// full five-number summary plus sample count, never a bare mean: a single scheduler hiccup
// drags a mean and nothing in that one number says so.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "bench_env.h"

namespace bench_report {

// Five-number summary (linear-interpolated quartiles, the same method spreadsheets call
// "inclusive"/R-7) plus sample count. relSpread() is the tool's own opinion of whether a
// run is trustworthy — see tooNoisy() below.
struct Stats
{
	double min = 0, q1 = 0, median = 0, q3 = 0, max = 0;
	int    n   = 0;

	[[nodiscard]] double iqr() const { return q3 - q1; }
	[[nodiscard]] double relSpread() const { return median > 0.0 ? iqr() / median : 0.0; }
};

namespace detail {

	// Linear-interpolated quantile at p in [0, 1] over an already-sorted vector.
	inline double quantile(const std::vector<double>& sorted, double p)
	{
		const auto n = sorted.size();
		if (n == 1)
			return sorted.front();
		const double index = p * static_cast<double>(n - 1);
		const auto   lower = static_cast<std::size_t>(std::floor(index));
		const auto   upper = static_cast<std::size_t>(std::ceil(index));
		const double frac  = index - static_cast<double>(lower);
		return sorted[lower] + frac * (sorted[upper] - sorted[lower]);
	}

} // namespace detail

// Sorts a copy of `samples` and reduces it to Stats. Throws std::invalid_argument on an
// empty input — every caller in this codebase controls the sample count itself, so an
// empty call is a caller bug, not a runtime condition to report gracefully.
[[nodiscard]] inline Stats computeStats(std::vector<double> samples)
{
	if (samples.empty())
		throw std::invalid_argument("computeStats: samples must not be empty");

	std::sort(samples.begin(), samples.end());

	Stats s;
	s.n      = static_cast<int>(samples.size());
	s.min    = samples.front();
	s.max    = samples.back();
	s.q1     = detail::quantile(samples, 0.25);
	s.median = detail::quantile(samples, 0.50);
	s.q3     = detail::quantile(samples, 0.75);
	return s;
}

// Nanoseconds per field cell per step — the metric that survives a change in problem size.
// `fieldCells` is the total cell count of every NeuralField in the architecture being
// timed (a multi-field synthetic benchmark sums across all N fields). Comparable ONLY
// within the same architecture family: the denominator counts field cells, but each field
// also drags a stimulus, kernel and noise element along with it, so the rate is silently
// wrong across architectures with a different element mix per field.
[[nodiscard]] inline double nsPerCellPerStep(double stepSeconds, long long fieldCells)
{
	if (fieldCells <= 0)
		throw std::invalid_argument("nsPerCellPerStep: fieldCells must be positive");
	return (stepSeconds * 1.0e9) / static_cast<double>(fieldCells);
}

// Above this relative spread (IQR / median), the run is inconclusive rather than merely
// noisy-looking: the tool should say so instead of leaving the reader to eyeball five
// numbers. 3% is tight on purpose — it is meant to fail on an unwrapped, un-pinned run and
// pass once tests/common's hygiene wrapper (affinity + priority + fixed clocks) is applied.
inline constexpr double kNoisyRelSpread = 0.03;

[[nodiscard]] inline bool tooNoisy(const Stats& s) { return s.relSpread() > kNoisyRelSpread; }

// One timed cell: a synthetic N-fields configuration (dnf_composer_benchmark) or one
// deck (dnf_composer_deckbench). `architecture` and `path` are free-form labels — e.g.
// "detection-2d-gauss" / "spectral-2d" — carried through to JSON so a later reader can
// tell whether two results are even comparable without re-deriving it from the name.
struct Result
{
	std::string name;
	std::string tier;
	std::string architecture;
	std::string path;
	long long   fieldCells = 0;
	bool        noisy      = false;
	Stats       stepsPerSec;
	Stats       nsPerCellStep;

	// FNV-1a64 of the source deck file's bytes (bench_env::detail::fnv1a64), empty for a
	// Result with no backing file (e.g. dnf_composer_benchmark's synthetic N-fields
	// configurations). A changed hash means a deck was edited, which invalidates any
	// baseline recorded against it regardless of what the timings say.
	std::string deckHash;
};

namespace detail {

	inline nlohmann::json statsToJson(const Stats& s)
	{
		return nlohmann::json{
			{"min", s.min}, {"q1", s.q1}, {"median", s.median},
			{"q3", s.q3}, {"max", s.max}, {"n", s.n},
		};
	}

	inline std::string isoTimestamp()
	{
		const std::time_t now = std::time(nullptr);
		std::tm tmv{};
#if defined(_WIN32)
		gmtime_s(&tmv, &now);
#else
		gmtime_r(&now, &tmv);
#endif
		char buf[32];
		std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tmv);
		return buf;
	}

} // namespace detail

// Schema-versioned JSON report: provenance (Env + fingerprint), the run configuration the
// caller supplies as opaque JSON (warmup/timed step counts, run count, ...), and every
// Result. "schema": 1 from the first version, so a baseline file read by a later tool
// build can tell it is looking at the shape it expects rather than guessing.
inline void writeJson(const std::string& path, const bench_env::Env& env,
                       const std::vector<Result>& results, const nlohmann::json& config)
{
	nlohmann::json j;
	j["schema"]      = 1;
	j["timestamp"]   = detail::isoTimestamp();
	j["fingerprint"] = bench_env::fingerprint(env);
	j["env"]         = bench_env::to_json(env);
	j["config"]      = config;

	auto& arr = j["results"];
	arr = nlohmann::json::array();
	for (const auto& r : results)
	{
		arr.push_back({
			{"name", r.name},
			{"tier", r.tier},
			{"architecture", r.architecture},
			{"path", r.path},
			{"field_cells", r.fieldCells},
			{"noisy", r.noisy},
			{"deck_hash", r.deckHash},
			{"steps_per_sec", detail::statsToJson(r.stepsPerSec)},
			{"ns_per_cell_step", detail::statsToJson(r.nsPerCellStep)},
		});
	}

	std::ofstream f(path);
	if (!f)
		throw std::runtime_error("bench_report::writeJson: cannot open " + path);
	f << j.dump(2) << '\n';
}

// Builds a Result from raw per-run step-time samples (seconds), computing both stats
// blocks and the noisy flag in one place so every caller reports identically.
[[nodiscard]] inline Result makeResult(std::string name, std::string tier,
                                        std::string architecture, std::string path,
                                        long long fieldCells,
                                        const std::vector<double>& stepSecondsSamples)
{
	Result r;
	r.name         = std::move(name);
	r.tier         = std::move(tier);
	r.architecture = std::move(architecture);
	r.path         = std::move(path);
	r.fieldCells   = fieldCells;

	std::vector<double> sps;
	std::vector<double> nsCell;
	sps.reserve(stepSecondsSamples.size());
	nsCell.reserve(stepSecondsSamples.size());
	for (const double stepSeconds : stepSecondsSamples)
	{
		sps.push_back(stepSeconds > 0.0 ? 1.0 / stepSeconds : 0.0);
		nsCell.push_back(nsPerCellPerStep(stepSeconds, fieldCells));
	}

	r.stepsPerSec   = computeStats(sps);
	r.nsPerCellStep = computeStats(nsCell);
	r.noisy         = tooNoisy(r.nsPerCellStep);
	return r;
}

} // namespace bench_report
