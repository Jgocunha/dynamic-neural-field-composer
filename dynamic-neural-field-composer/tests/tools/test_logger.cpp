#include <gtest/gtest.h>
#include <string>

#include "tools/logger.h"

using namespace dnf_composer::tools::logger;

// ---------------------------------------------------------------------------
// Logger construction
// ---------------------------------------------------------------------------

TEST(LoggerTest, ConstructWithEachLevelDoesNotThrow)
{
    EXPECT_NO_THROW(Logger(LogLevel::DEBUG,   LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(Logger(LogLevel::INFO,    LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(Logger(LogLevel::WARNING, LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(Logger(LogLevel::ERROR,   LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(Logger(LogLevel::FATAL,   LogOutputMode::CONSOLE));
}

// ---------------------------------------------------------------------------
// Logger::log — console mode (safe without an ImGui context)
// ---------------------------------------------------------------------------

TEST(LoggerTest, LogDoesNotThrow)
{
    Logger l(LogLevel::INFO, LogOutputMode::CONSOLE);
    EXPECT_NO_THROW(l.log("test message"));
}

TEST(LoggerTest, LogProducesOutput)
{
    Logger l(LogLevel::INFO, LogOutputMode::CONSOLE);
    ::testing::internal::CaptureStdout();
    l.log("hello coverage");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_FALSE(out.empty());
}

TEST(LoggerTest, LogOutputContainsMessage)
{
    Logger l(LogLevel::WARNING, LogOutputMode::CONSOLE);
    ::testing::internal::CaptureStdout();
    l.log("unique-marker-string");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("unique-marker-string"), std::string::npos);
}

// ---------------------------------------------------------------------------
// setMinLogLevel — messages below the threshold are suppressed
// ---------------------------------------------------------------------------

TEST(LoggerTest, MessageBelowMinLogLevelIsNotPrinted)
{
    Logger::setMinLogLevel(LogLevel::ERROR);
    Logger l(LogLevel::DEBUG, LogOutputMode::CONSOLE);
    ::testing::internal::CaptureStdout();
    l.log("should be suppressed");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_TRUE(out.empty());
    Logger::setMinLogLevel(LogLevel::DEBUG); // restore
}

TEST(LoggerTest, MessageAtMinLogLevelIsPrinted)
{
    Logger::setMinLogLevel(LogLevel::WARNING);
    Logger l(LogLevel::WARNING, LogOutputMode::CONSOLE);
    ::testing::internal::CaptureStdout();
    l.log("at-threshold");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("at-threshold"), std::string::npos);
    Logger::setMinLogLevel(LogLevel::DEBUG); // restore
}

TEST(LoggerTest, MessageAboveMinLogLevelIsPrinted)
{
    Logger::setMinLogLevel(LogLevel::WARNING);
    Logger l(LogLevel::FATAL, LogOutputMode::CONSOLE);
    ::testing::internal::CaptureStdout();
    l.log("above-threshold");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("above-threshold"), std::string::npos);
    Logger::setMinLogLevel(LogLevel::DEBUG); // restore
}

// ---------------------------------------------------------------------------
// Free function log()
// ---------------------------------------------------------------------------

TEST(LoggerTest, FreeFunctionLogDoesNotThrow)
{
    EXPECT_NO_THROW(log(LogLevel::INFO,    "free info",    LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(log(LogLevel::WARNING, "free warning", LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(log(LogLevel::ERROR,   "free error",   LogOutputMode::CONSOLE));
    EXPECT_NO_THROW(log(LogLevel::FATAL,   "free fatal",   LogOutputMode::CONSOLE));
}

TEST(LoggerTest, FreeFunctionLogProducesOutput)
{
    ::testing::internal::CaptureStdout();
    log(LogLevel::INFO, "free-fn-marker", LogOutputMode::CONSOLE);
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("free-fn-marker"), std::string::npos);
}
