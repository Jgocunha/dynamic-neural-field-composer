#include "tools/logger.h"

#include <mutex>

namespace dnf_composer::tools::logger
{
    std::atomic<LogLevel> Logger::minLogLevel = LogLevel::DEBUG;

    namespace
    {
        // Serializes the actual emit (console write / GUI push_back) so concurrent
        // log() calls from worker threads don't interleave or corrupt the sinks.
        // Formatting happens outside the lock; only the write is guarded. Also
        // guards writes/reads of uiSink() below, since setUiSink() and log_ui()
        // can race the same way.
        std::mutex& logSinkMutex()
        {
            static std::mutex m;
            return m;
        }

        // The UI-registered callback for GUI-destined messages. Empty until the UI
        // layer calls Logger::setUiSink() (normally once, at startup) -- log_ui()
        // is a no-op until then, which is exactly what keeps tools/ (and headless
        // test builds) free of any GUI/application dependency.
        UiLogSink& uiSink()
        {
            static UiLogSink sink;
            return sink;
        }
    }

    Logger::Logger(const LogLevel level, const LogOutputMode mode)
        : logLevel(level), outputMode(mode)
    {}


    void Logger::log(const std::string& message) const
    {
        if (logLevel < Logger::minLogLevel.load(std::memory_order_relaxed)) {
            return;
        }

        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm buf;
        if (!utils::safe_localtime(&in_time_t, &buf)) {
            throw Exception(ErrorCode::LOG_LOCAL_TIME_ERROR);
}

        const std::string levelStr = getLogLevelText(logLevel);
        const std::string prefixStr = "<dnf-composer> " + levelStr;

        switch (outputMode)
        {
        case LogOutputMode::ALL:
            {
                // Console output
                std::ostringstream consoleOss;
                std::string colorCode = getLogLevelColorCodeCmd(logLevel);
                consoleOss << colorCode << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;

                // GUI output (separate stringstream)
                std::ostringstream guiOss;
                guiOss << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;

                {
                    std::lock_guard<std::mutex> lock(logSinkMutex());
                    log_cmd(consoleOss.str());
                }
                log_ui(logLevel, guiOss.str());
            }
            break;
        case LogOutputMode::CONSOLE:
            {
                std::ostringstream oss;
                std::string colorCode = getLogLevelColorCodeCmd(logLevel);
                oss << colorCode << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;

                std::lock_guard<std::mutex> lock(logSinkMutex());
                log_cmd(oss.str());
            }
            break;
        case LogOutputMode::GUI:
            {
                std::ostringstream oss;
                oss << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;

                log_ui(logLevel, oss.str());
            }
            break;
        default:
            break;
        }
    }

    void Logger::log_cmd(const std::string& message)
    {
        const std::string finalMessage_cmd = message + "\033[0m"; // Reset color code
        std::cout << finalMessage_cmd << '\n';
    }

    void Logger::log_ui(const LogLevel level, const std::string& message)
    {
        // Copy the sink and release logSinkMutex before invoking it: the sink
        // runs arbitrary UI code, and if that code calls setUiSink() reentrantly
        // (e.g. to unregister itself), calling it while still holding the lock
        // would deadlock on the non-recursive mutex.
        UiLogSink sink;
        {
            std::lock_guard<std::mutex> lock(logSinkMutex());
            sink = uiSink();
        }
        if (sink) {
            sink(level, message);
        }
    }

    void Logger::setUiSink(UiLogSink sink)
    {
        std::lock_guard<std::mutex> lock(logSinkMutex());
        uiSink() = std::move(sink);
    }

    void log(const LogLevel level, const std::string& message, const LogOutputMode mode)
    {
#ifndef _DEBUG
        if (level == LogLevel::DEBUG) {
            return;
}
#endif

        // Use a local Logger (no shared mutable global) so concurrent log() calls
        // from different threads don't race on a shared instance. The sinks
        // themselves are serialized inside Logger::log().
        Logger(level, mode).log(message);
    }

    std::string Logger::getLogLevelColorCodeCmd(const LogLevel level)
    {
        switch (level)
        {
        case DEBUG:     return "\033[92m"; // Green
        case INFO:      return"\033[0m";
        case WARNING:   return"\033[93m";  // Yellow
        case ERROR:
        case FATAL:     return"\033[91m";  // Red
        default:        return "\033[0m";
        }
    }

    std::string Logger::getLogLevelText(const LogLevel level)
    {
        switch (level)
        {
        case DEBUG: return      "DEBUG   ";
        case INFO: return       "INFO    ";
        case WARNING: return    "WARNING ";
        case ERROR: return      "ERROR   ";
        case FATAL: return      "FATAL   ";
        default: return         "UNKNOWN ";
        }
    }
}
