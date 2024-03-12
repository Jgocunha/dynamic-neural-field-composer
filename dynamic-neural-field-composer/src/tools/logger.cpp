// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "tools/logger.h"

namespace dnf_composer
{

    namespace tools
    {
        namespace logger
        {
            Logger::Logger(LogLevel level, LogOutputMode mode)
                : logLevel(level), outputMode(mode)
            {}

            void Logger::log(const std::string& message) const
            {
                const auto now = std::chrono::system_clock::now();
                const auto in_time_t = std::chrono::system_clock::to_time_t(now);

                std::tm buf;
                //if (localtime_s(&buf, &in_time_t))
                    //throw dnf_composer::Exception(dnf_composer::ErrorCode::LOG_LOCAL_TIME_ERROR);

                const std::string levelStr = getLogLevelText(logLevel);
                std::ostringstream oss;
                const std::string finalMessage = oss.str();
                std::string colorCode;

                switch (outputMode)
                {
                case LogOutputMode::ALL:
                    colorCode = getLogLevelColorCode(logLevel);
                    oss << colorCode << levelStr << " " << std::put_time(&buf, "%Y-%m-%d %X") << " " << message;
                    log_cmd(oss.str());
                    std::ostringstream().swap(oss); // swap m with a default constructed stringstream
                    oss << levelStr << " " << std::put_time(&buf, "%Y-%m-%d %X") << " " << message;
                    log_ui(oss.str());
                    break;
                case LogOutputMode::CONSOLE:
                    colorCode = getLogLevelColorCode(logLevel);
                    oss << colorCode << levelStr << " " << std::put_time(&buf, "%Y-%m-%d %X") << " " << message;
                    log_cmd(oss.str());
                    break;
                case LogOutputMode::GUI:
                    oss << levelStr << " " << std::put_time(&buf, "%Y-%m-%d %X") << " " << message;
                    log_ui(oss.str());
                    break;
                default:
                    break;
                }

            }

            void Logger::log_cmd(const std::string& message)
            {
                const std::string finalMessage_cmd = message + "\033[0m"; // Reset color code
                std::cout << finalMessage_cmd;
            }

            void Logger::log_ui(const std::string& message)
            {
                dnf_composer::user_interface::LoggerWindow::addLog(message.c_str());
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


            std::string Logger::getLogLevelColorCode(LogLevel level)
            {
                switch (level)
                {
                case DEBUG:
                    return "\033[92m";   // Green
                case INFO:
                    return"\033[97m";   // White
                case WARNING:
                    return"\033[93m"; // Yellow
                case ERROR:
                case FATAL:
                    return"\033[91m"; // Red
                default:
                    return"";
                }
            }

            std::string Logger::getLogLevelText(LogLevel level)
            {
                switch (level)
                {
                case DEBUG: return      "[DEBUG]  ";
                case INFO: return       "[INFO]   ";
                case WARNING: return    "[WARNING]";
                case ERROR: return      "[ERROR]  ";
                case FATAL: return      "[FATAL]  ";
                default: return "";
                }
            }
        }
    }
}