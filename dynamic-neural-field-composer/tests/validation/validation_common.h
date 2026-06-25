#pragma once

// Shared helpers for the field-dynamics regression validation suite.
//
// These re-run the cross-platform-validation protocol (500 steps stimulus ON,
// then 500 steps stimulus OFF) against the LIVE dnf-composer library and compare
// the resulting "neural field u" activation profiles to the vendored reference
// CSVs under tests/validation/data/. They guard that optimizations do not alter
// field dynamics. Reference CSVs store ~6 significant figures, so the comparison
// tolerance is an absolute 1e-4.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "simulation/simulation.h"
#include "simulation/simulation_file_manager.h"
#include "elements/gauss_stimulus.h"
#include "elements/gauss_stimulus_2d.h"
#include "tools/logger.h"

namespace dnf_composer::test_validation
{
	namespace fs = std::filesystem;

	// Absolute tolerance — the reference CSVs are stored with ~6 significant
	// figures, so this is the tightest meaningful element-wise bound.
	inline constexpr double kAbsTolerance = 1e-4;

	inline constexpr int kStepsPhase1 = 500; // stimulus ON
	inline constexpr int kStepsPhase2 = 500; // stimulus OFF

	// Root of the vendored validation data (set by CMake via target_compile_definitions).
#ifndef VALIDATION_DATA_DIR
#define VALIDATION_DATA_DIR ""
#endif

	// Silence the per-element INFO logging so it does not drown the test output
	// (each sim load logs several lines; 300 sims => thousands of lines).
	inline void silenceLogging()
	{
		tools::logger::Logger::setMinLogLevel(tools::logger::LogLevel::FATAL);
	}

	struct ProtocolResult
	{
		std::vector<double> with_stimulus;
		std::vector<double> without_stimulus;
	};

	// Read a single-line, comma-separated CSV of doubles.
	inline std::vector<double> loadCsv(const fs::path& path)
	{
		std::ifstream f(path);
		if (!f) throw std::runtime_error("Cannot open reference CSV: " + path.string());
		std::vector<double> values;
		std::string token;
		while (std::getline(f, token, ','))
		{
			// Trim a trailing newline on the last field.
			if (!token.empty() && (token.back() == '\n' || token.back() == '\r'))
				token.pop_back();
			if (!token.empty())
				values.push_back(std::stod(token));
		}
		return values;
	}

	// Set amplitude=0 for every GaussStimulus / GaussStimulus2D in the simulation.
	inline void zeroAllStimuli(const std::shared_ptr<Simulation>& sim)
	{
		for (auto& elem : sim->getElements())
		{
			if (auto s1 = std::dynamic_pointer_cast<element::GaussStimulus>(elem))
			{
				auto p = s1->getParameters();
				p.amplitude = 0.0;
				s1->setParameters(p);
			}
			else if (auto s2 = std::dynamic_pointer_cast<element::GaussStimulus2D>(elem))
			{
				auto p = s2->getParameters();
				p.amplitude = 0.0;
				s2->setParameters(p);
			}
		}
	}

	// Run the two-phase protocol against the live library for one sim JSON.
	inline ProtocolResult runProtocol(const fs::path& jsonPath)
	{
		const std::string stem = jsonPath.stem().string();
		auto sim = std::make_shared<Simulation>(stem);
		const SimulationFileManager sfm(sim, jsonPath.string());
		sfm.loadElementsFromJson();

		sim->init();
		for (int t = 0; t < kStepsPhase1; ++t) sim->step();
		ProtocolResult r;
		r.with_stimulus = sim->getComponent("neural field u", "activation");

		zeroAllStimuli(sim);
		for (int t = 0; t < kStepsPhase2; ++t) sim->step();
		r.without_stimulus = sim->getComponent("neural field u", "activation");

		return r;
	}

	// Compare actual vs expected element-wise; returns the max absolute deviation.
	// On size mismatch returns infinity (always fails the tolerance assertion).
	inline double maxAbsDeviation(const std::vector<double>& actual,
	                              const std::vector<double>& expected)
	{
		if (actual.size() != expected.size())
			return std::numeric_limits<double>::infinity();
		double maxDev = 0.0;
		for (std::size_t i = 0; i < actual.size(); ++i)
			maxDev = std::max(maxDev, std::abs(actual[i] - expected[i]));
		return maxDev;
	}

	// Find the first index whose deviation exceeds the tolerance (for diagnostics).
	inline std::ptrdiff_t firstViolation(const std::vector<double>& actual,
	                                      const std::vector<double>& expected, double tol)
	{
		if (actual.size() != expected.size()) return 0;
		for (std::size_t i = 0; i < actual.size(); ++i)
			if (std::abs(actual[i] - expected[i]) > tol)
				return static_cast<std::ptrdiff_t>(i);
		return -1;
	}

	// Enumerate the sim JSON stems for a dimension ("1d" or "2d").
	inline std::vector<std::string> collectSimStems(const std::string& dim)
	{
		std::vector<std::string> stems;
		try
		{
			const fs::path simDir = fs::path(VALIDATION_DATA_DIR) / dim / "simulations";
			std::error_code ec;
			if (!fs::exists(simDir, ec) || ec) return stems;
			for (const auto& entry : fs::directory_iterator(simDir, ec))
			{
				if (ec) break;
				if (entry.path().extension() == ".json")
					stems.push_back(entry.path().stem().string());
			}
		}
		catch (...) { /* static-init safe: return whatever we have */ }
		std::sort(stems.begin(), stems.end());
		return stems;
	}

	inline fs::path simJsonPath(const std::string& dim, const std::string& stem)
	{
		return fs::path(VALIDATION_DATA_DIR) / dim / "simulations" / (stem + ".json");
	}

	inline fs::path expectedCsvPath(const std::string& dim, const std::string& stem,
	                                bool withStimulus)
	{
		const std::string suffix = withStimulus ? "_with_stimulus.csv" : "_without_stimulus.csv";
		return fs::path(VALIDATION_DATA_DIR) / dim / "expected" / (stem + suffix);
	}

	// Run one sim and assert both phases match the vendored reference within tol.
	inline void expectSimMatchesReference(const std::string& dim, const std::string& stem)
	{
		const ProtocolResult got = runProtocol(simJsonPath(dim, stem));

		const auto expWith    = loadCsv(expectedCsvPath(dim, stem, true));
		const auto expWithout = loadCsv(expectedCsvPath(dim, stem, false));

		const double devWith    = maxAbsDeviation(got.with_stimulus, expWith);
		const double devWithout = maxAbsDeviation(got.without_stimulus, expWithout);

		EXPECT_LE(devWith, kAbsTolerance)
			<< dim << " " << stem << " [with_stimulus] max abs dev=" << devWith
			<< " first violation at index "
			<< firstViolation(got.with_stimulus, expWith, kAbsTolerance)
			<< " (actual size=" << got.with_stimulus.size()
			<< ", expected size=" << expWith.size() << ")";

		EXPECT_LE(devWithout, kAbsTolerance)
			<< dim << " " << stem << " [without_stimulus] max abs dev=" << devWithout
			<< " first violation at index "
			<< firstViolation(got.without_stimulus, expWithout, kAbsTolerance)
			<< " (actual size=" << got.without_stimulus.size()
			<< ", expected size=" << expWithout.size() << ")";
	}
}
