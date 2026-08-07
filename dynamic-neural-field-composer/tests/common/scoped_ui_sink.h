#pragma once

#include "tools/logger.h"

namespace dnf_composer::test
{
	/// @brief RAII guard that installs a UI log sink for the current scope and
	///        clears it on destruction.
	///
	/// `Logger`'s UI sink is process-wide state, same as the min-log-level
	/// threshold (see `ScopedMinLogLevel`): a test that registers a fake sink and
	/// never clears it would leak that callback into every test that runs
	/// afterwards. This binary never constructs an `Application` (headless tests
	/// must not touch the GUI), so the sink is unregistered by default -- clearing
	/// it back to empty on scope exit restores exactly that default.
	class ScopedUiSink
	{
	public:
		/// @brief Register @p sink for the current scope.
		/// @param sink Callback to install until this object goes out of scope.
		explicit ScopedUiSink(tools::logger::UiLogSink sink)
		{
			tools::logger::Logger::setUiSink(std::move(sink));
		}

		~ScopedUiSink()
		{
			tools::logger::Logger::setUiSink(tools::logger::UiLogSink{});
		}

		ScopedUiSink(const ScopedUiSink&) = delete;
		ScopedUiSink& operator=(const ScopedUiSink&) = delete;
		ScopedUiSink(ScopedUiSink&&) = delete;
		ScopedUiSink& operator=(ScopedUiSink&&) = delete;
	};
}
