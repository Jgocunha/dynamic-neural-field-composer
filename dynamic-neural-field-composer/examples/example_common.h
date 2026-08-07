#pragma once

// Shared helper for examples/*.cpp: the standard GUI example main loop, with an
// optional headless short-run mode for CI smoke-testing (#129).
//
// The examples are meant to be run interactively and normally block until the
// user closes the window, which is exactly why they were never exercised in
// CI: nothing closes the window for you. Setting DNFC_EXAMPLE_MAX_STEPS to a
// positive integer makes runExampleLoop() call Application::requestQuit()
// once that many app.step() calls have run, so the process exits 0 on its
// own. Left unset (the normal interactive case), the loop behaves exactly as
// it always has -- it only returns when the user closes the window.

#include <cstdlib>
#include <optional>

#include "application/application.h"

namespace dnf_composer::examples
{
	/// @brief Read DNFC_EXAMPLE_MAX_STEPS from the environment.
	/// @return The parsed step count, or std::nullopt if unset/empty/invalid.
	[[nodiscard]] inline std::optional<long long> headlessMaxSteps()
	{
		const char* env = std::getenv("DNFC_EXAMPLE_MAX_STEPS");
		if (!env || *env == '\0')
			return std::nullopt;

		char* end = nullptr;
		const long long value = std::strtoll(env, &end, 10);
		if (end == env || value <= 0)
			return std::nullopt;

		return value;
	}

	/// @brief Run the standard example main loop: step until the GUI is closed.
	///
	/// If DNFC_EXAMPLE_MAX_STEPS is set in the environment, the application
	/// requests its own shutdown after that many steps instead of waiting
	/// indefinitely for a window close. Used by the CI headless smoke-run step
	/// (see .github/workflows/ci.yml) and by anyone smoke-testing an example
	/// from the command line without wanting to close a window by hand.
	inline void runExampleLoop(const Application& app)
	{
		const std::optional<long long> maxSteps = headlessMaxSteps();
		long long stepsRun = 0;

		while (!app.hasGUIBeenClosed())
		{
			app.step();
			if (maxSteps.has_value() && ++stepsRun >= *maxSteps)
				Application::requestQuit();
		}
	}
}
