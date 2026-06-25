#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <imgui-platform-kit/log_window.h>

#include "exceptions/exception.h"
#include "utils.h"

#ifdef ERROR
#undef ERROR
#endif



namespace dnf_composer::tools::logger
{
	enum LogLevel : int
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		FATAL
	};

	enum LogOutputMode : int
	{
		CONSOLE,
		GUI,
		ALL
	};

	class Logger
	{
	private:
		LogLevel logLevel;
		LogOutputMode outputMode;
		// Atomic so setMinLogLevel() (often called from a UI thread) and the load in
		// log() (called from any worker thread) don't race. See logger.cpp.
		static std::atomic<LogLevel> minLogLevel;
	public:
		Logger(LogLevel level, LogOutputMode mode = ALL);
		void log(const std::string& message) const;
		static void setMinLogLevel(LogLevel level) { minLogLevel.store(level, std::memory_order_relaxed); }
	private:
		static std::string getLogLevelColorCodeCmd(LogLevel level);
		static ImVec4 getLogLevelColorCodeGui(LogLevel level);
		static std::string getLogLevelText(LogLevel level);
		static void log_cmd(const std::string& message);
		static void log_ui(ImVec4 color, const std::string& message);
	};

	void log(LogLevel level, const std::string& message, LogOutputMode mode = ALL);
}



