//This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "tools/logger.h"

namespace dnf_composer
{

    namespace tools
    {
        namespace logger
        {
            LogLevel Logger::minLogLevel = LogLevel::DEBUG;

            Logger::Logger(LogLevel level, LogOutputMode mode)
                : logLevel(level), outputMode(mode)
            {}

            void Logger::log(const std::string& message) const
            {
                if (logLevel < Logger::minLogLevel)
                    return;

                const auto now = std::chrono::system_clock::now();
                const auto in_time_t = std::chrono::system_clock::to_time_t(now);

                std::tm buf;
                if (localtime_s(&buf, &in_time_t))
                    throw std::runtime_error("Failed to get current time for logging.");

                const std::string levelStr = getLogLevelText(logLevel);
                const std::string prefixStr = "<dnf-composer> " + levelStr;
                std::ostringstream oss;
                const std::string finalMessage = oss.str();
                std::string colorCode;

                const ImVec4 color = getLogLevelColorCodeGui(logLevel);
                switch (outputMode)
                {
                case LogOutputMode::ALL:
                    colorCode = getLogLevelColorCodeCmd(logLevel);
                    oss << colorCode << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;
                    log_cmd(oss.str());
                    std::ostringstream().swap(oss); // swap m with a default constructed stringstream
                    oss << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << " " << message;
                    log_ui(color, oss.str());
                    break;
                case LogOutputMode::CONSOLE:
                    colorCode = getLogLevelColorCodeCmd(logLevel);
                    oss << colorCode << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << message;
                    log_cmd(oss.str());
                    break;
                case LogOutputMode::GUI:
                    oss << "[" << std::put_time(&buf, "%Y-%m-%d %X") << "] " << prefixStr << " " << " " << message;
                    log_ui(color, oss.str());
                    break;
                default:
                    break;
                }

            }

            void Logger::log_cmd(const std::string& message)
            {
                const std::string finalMessage_cmd = message + "\033[0m"; // Reset color code
                std::cout << finalMessage_cmd << std::endl;
            }

            void Logger::log_ui(ImVec4 color, const std::string& message)
            {
                imgui_kit::LogWindow::addLog(color, message.c_str());
            }

            void log(LogLevel level, const std::string& message, LogOutputMode mode)
            {
#ifndef _DEBUG
                if (level == LogLevel::DEBUG)
                    return;
#endif

                logger = Logger(level, mode);
                logger.log(message);
            }

            std::string Logger::getLogLevelColorCodeCmd(LogLevel level)
            {
                switch (level)
                {
                case DEBUG:     return "\033[92m"; // Green
                case INFO:      return"\033[97m";  // White
                case WARNING:   return"\033[93m";  // Yellow
                case ERROR:
                case FATAL:     return"\033[91m";  // Red
                default:        return "\033[97m"; // White
                }
            }

            ImVec4 Logger::getLogLevelColorCodeGui(LogLevel level)
            {
                switch (level)
                {
                case DEBUG:     return imgui_kit::colours::Green;  // Green
                case INFO:      return imgui_kit::colours::White;  // White
                case WARNING:   return imgui_kit::colours::Yellow; // Yellow
                case ERROR:
                case FATAL:     return imgui_kit::colours::Red;    // Red
                default:        return imgui_kit::colours::White;  // White
                }
            }

            std::string Logger::getLogLevelText(LogLevel level)
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
    }
}