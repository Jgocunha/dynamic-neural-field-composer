#pragma once

#include "tools/logger.h"

namespace dnf_composer::test
{
	/// @brief RAII guard that temporarily changes the global logger threshold
	///        and restores the previous value on destruction.
	///
	/// `Logger::minLogLevel` is process-wide global state. A test that raises it
	/// to silence noisy output and never restores it leaks that change into every
	/// test that runs afterwards, silently suppressing their log output — which
	/// makes any test asserting on logger output fail depending only on the order
	/// the suites happen to run in. Use this guard instead of a bare
	/// `setMinLogLevel()` call so the restore cannot be skipped, including when a
	/// test fails or throws part-way through.
	class ScopedMinLogLevel
	{
	public:
		/// @brief Raise/lower the global threshold for the current scope.
		/// @param level Threshold to apply until this object goes out of scope.
		explicit ScopedMinLogLevel(const tools::logger::LogLevel level)
			: previous(tools::logger::Logger::getMinLogLevel())
		{
			tools::logger::Logger::setMinLogLevel(level);
		}

		~ScopedMinLogLevel()
		{
			tools::logger::Logger::setMinLogLevel(previous);
		}

		ScopedMinLogLevel(const ScopedMinLogLevel&) = delete;
		ScopedMinLogLevel& operator=(const ScopedMinLogLevel&) = delete;
		ScopedMinLogLevel(ScopedMinLogLevel&&) = delete;
		ScopedMinLogLevel& operator=(ScopedMinLogLevel&&) = delete;

	private:
		tools::logger::LogLevel previous;
	};
}
