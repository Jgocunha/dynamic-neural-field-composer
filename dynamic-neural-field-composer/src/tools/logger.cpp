#include "tools/logger.h"

#include <mutex>

#include "application/application.h"
#include "user_interface/log_window.h"

namespace dnf_composer::tools::logger
{
    std::atomic<LogLevel> Logger::minLogLevel = LogLevel::DEBUG;

    namespace
    {
        // Serializes the actual emit (console write / GUI push_back) so concurrent
        // log() calls from worker threads don't interleave or corrupt the sinks.
        // Formatting happens outside the lock; only the write is guarded.
        std::mutex& logSinkMutex()
        {
            static std::mutex m;
            return m;
        }
    }

    Logger::Logger(const LogLevel level, const LogOutputMode mode)
        : logLevel(level), outputMode(mode)
    {}


    void Logger::log(const std::string& message) const
    {
        if (logLevel < Logger::minLogLevel.load(std::memory_order_relaxed))
            return;

        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm buf;
        if (!utils::safe_localtime(&in_time_t, &buf))
            throw Exception(ErrorCode::LOG_LOCAL_TIME_ERROR);

        const std::string levelStr = getLogLevelText(logLevel);
        const std::string prefixStr = "<dnf-composer> " + levelStr;
        const ImVec4 color = getLogLevelColorCodeGui(logLevel);

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

                std::lock_guard<std::mutex> lock(logSinkMutex());
                log_cmd(consoleOss.str());
                log_ui(color, guiOss.str());
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

                std::lock_guard<std::mutex> lock(logSinkMutex());
                log_ui(color, oss.str());
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

    void Logger::log_ui(const ImVec4 color, const std::string& message)
    {
        user_interface::LogWindow::addLog(color, "%s", message.c_str());
    }

    void log(const LogLevel level, const std::string& message, const LogOutputMode mode)
    {
#ifndef _DEBUG
        if (level == LogLevel::DEBUG)
            return;
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

    ImVec4 Logger::getLogLevelColorCodeGui(const LogLevel level)
    {
        ImVec4 currentTextColor = imgui_kit::colours::Gray;
        if (ImGui::GetCurrentContext())
        {
            const ImGuiStyle& style = ImGui::GetStyle();
            currentTextColor = style.Colors[ImGuiCol_Text];
        }

        switch (level)
        {
        case DEBUG:     return imgui_kit::colours::Green;
        case INFO:      return imgui_kit::colours::White;
        case WARNING:   return imgui_kit::colours::Yellow;
        case ERROR:
        case FATAL:     return imgui_kit::colours::Red;
        default:        return currentTextColor;
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
