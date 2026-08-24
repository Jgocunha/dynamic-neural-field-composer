#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <functional>

#include "exceptions/exception.h"
#include "utils.h"

#ifdef ERROR
#undef ERROR
#endif



namespace dnf_composer::tools::logger
{
	/// @brief Severity of a log message, from most to least verbose.
	enum LogLevel : int
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		FATAL
	};

	/// @brief Where a log message should be delivered.
	enum LogOutputMode : int
	{
		CONSOLE,
		GUI,
		ALL
	};

	/// @brief Callback the UI layer registers to receive GUI-destined log messages.
	///
	/// Decouples tools/ from the GUI: instead of the logger calling into a concrete
	/// UI type directly, the UI hands the logger a callback at startup and the
	/// logger just invokes it. @p message is the fully formatted log line
	/// (timestamp + level prefix + text); @p level lets the callback choose a
	/// color/style without tools/ having to know anything about ImGui.
	using UiLogSink = std::function<void(LogLevel level, const std::string& message)>;

	/// @brief Formats and dispatches a single log message to console and/or GUI.
	class Logger
	{
	private:
		LogLevel logLevel;
		LogOutputMode outputMode;
		// Atomic so setMinLogLevel() (often called from a UI thread) and the load in
		// log() (called from any worker thread) don't race. See logger.cpp.
		static std::atomic<LogLevel> minLogLevel;
	public:
		/// @brief Construct a logger for one message.
		/// @param level Severity of the message.
		/// @param mode  Where the message should be delivered.
		Logger(LogLevel level, LogOutputMode mode = ALL);
		/// @brief Emit @p message if @p level is at or above the global minimum level.
		/// @param message Text to log.
		void log(const std::string& message) const;
		/// @brief Set the global minimum level; messages below it are dropped.
		/// @param level New global threshold.
		static void setMinLogLevel(LogLevel level) { minLogLevel.store(level, std::memory_order_relaxed); }
		/// @brief Read the global minimum level.
		/// @return The current global threshold.
		/// @note Provided so callers that temporarily raise the threshold can
		///       restore the previous value instead of guessing the default.
		static LogLevel getMinLogLevel() { return minLogLevel.load(std::memory_order_relaxed); }

		/// @brief Register the callback that receives GUI-destined log messages.
		///
		/// Call once at application/UI startup (see @c Application::init()). Until a
		/// sink is registered, GUI-mode logging is a no-op — this is what lets
		/// tools/ (and headless/test builds that never construct a UI) link and run
		/// without any GUI/application dependency.
		/// @param sink Callback invoked from GUI-destined log calls. Pass an empty
		///             @c std::function to clear the registration (e.g. in tests).
		static void setUiSink(UiLogSink sink);
	private:
		static std::string getLogLevelColorCodeCmd(LogLevel level);
		static std::string getLogLevelText(LogLevel level);
		static void log_cmd(const std::string& message);
		/// @brief Forward a formatted message to the registered UI sink, if any.
		/// @param level   Severity of the message.
		/// @param message Fully formatted log line.
		/// @note No-op when no sink is registered (see @c setUiSink()).
		static void log_ui(LogLevel level, const std::string& message);
	};

	/// @brief Convenience free function: build a @c Logger and log @p message with it.
	/// @param level   Severity of the message.
	/// @param message Text to log.
	/// @param mode    Where the message should be delivered.
	void log(LogLevel level, const std::string& message, LogOutputMode mode = ALL);
}



