#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "tools/logger.h"
#include "scoped_min_log_level.h"
#include "scoped_ui_sink.h"

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

// ---------------------------------------------------------------------------
// ScopedMinLogLevel — the global threshold must not leak out of a scope.
// Regression guard: tests that raise the level to stay quiet previously left it
// raised, silently suppressing the output every later suite asserted on, so the
// suite passed or failed purely on registration order.
// ---------------------------------------------------------------------------

TEST(LoggerTest, ScopedMinLogLevelRestoresPreviousLevel)
{
    // Restore whatever the threshold was on entry -- these tests must not leak
    // their own DEBUG setting either, which is the very bug they guard against.
    const dnf_composer::test::ScopedMinLogLevel restoreOnExit{ Logger::getMinLogLevel() };
    Logger::setMinLogLevel(LogLevel::DEBUG);
    {
        const dnf_composer::test::ScopedMinLogLevel quiet{ LogLevel::FATAL };
        EXPECT_EQ(Logger::getMinLogLevel(), LogLevel::FATAL);
    }
    EXPECT_EQ(Logger::getMinLogLevel(), LogLevel::DEBUG);
}

TEST(LoggerTest, ScopedMinLogLevelRestoresOnException)
{
    const dnf_composer::test::ScopedMinLogLevel restoreOnExit{ Logger::getMinLogLevel() };
    Logger::setMinLogLevel(LogLevel::DEBUG);
    try
    {
        const dnf_composer::test::ScopedMinLogLevel quiet{ LogLevel::FATAL };
        throw std::runtime_error("boom");
    }
    catch (const std::runtime_error&) { /* expected */ }
    EXPECT_EQ(Logger::getMinLogLevel(), LogLevel::DEBUG);
}

// A message at INFO must still reach the console after a "quiet" scope ends.
// This is the exact assertion that order-dependent leakage used to break.
TEST(LoggerTest, LogOutputSurvivesAQuietScope)
{
    const dnf_composer::test::ScopedMinLogLevel restoreOnExit{ Logger::getMinLogLevel() };
    Logger::setMinLogLevel(LogLevel::DEBUG);
    {
        const dnf_composer::test::ScopedMinLogLevel quiet{ LogLevel::FATAL };
        Logger(LogLevel::INFO, LogOutputMode::CONSOLE).log("suppressed");
    }
    ::testing::internal::CaptureStdout();
    Logger(LogLevel::INFO, LogOutputMode::CONSOLE).log("visible-after-scope");
    const std::string out = ::testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("visible-after-scope"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Dependency direction (issue #123): tools/logger.h must compile without any
// GUI/application include. Logger::log_ui() reaches the UI through a
// registered sink (below) instead of calling into user_interface/log_window.h
// or application/application.h directly -- tools/ is a low-level layer and
// must not depend on the GUI layer that sits above it. The strongest proof of
// this is that this whole binary links (test_logger.cpp itself never includes
// anything GUI-related); this also asserts it directly against the header
// source, so a regression fails loudly instead of only showing up as a build
// break days later.
// ---------------------------------------------------------------------------

TEST(LoggerTest, HeaderHasNoGuiOrApplicationInclude)
{
    std::ifstream header(LOGGER_HEADER_PATH);
    ASSERT_TRUE(header.is_open()) << "Could not open " << LOGGER_HEADER_PATH;

    std::ostringstream contents;
    contents << header.rdbuf();
    const std::string text = contents.str();

    EXPECT_EQ(text.find("application/application.h"), std::string::npos);
    EXPECT_EQ(text.find("user_interface/log_window.h"), std::string::npos);
    EXPECT_EQ(text.find("imgui-platform-kit"), std::string::npos);
    EXPECT_EQ(text.find("imgui.h"), std::string::npos);
}

// ---------------------------------------------------------------------------
// UI sink registration (issue #123) -- the mechanism that replaces the direct
// call into user_interface::LogWindow. Guarded by ScopedUiSink so a fake sink
// registered here cannot leak into a later test (see scoped_ui_sink.h).
// ---------------------------------------------------------------------------

TEST(LoggerTest, NoUiSinkRegisteredIsNoop)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };

    // Default state for this headless test binary: nothing has registered a
    // sink (that only happens in Application::init(), which tests never call).
    // GUI-mode logging must be a silent no-op, not a crash or a GUI call.
    EXPECT_NO_THROW(Logger(LogLevel::INFO, LogOutputMode::GUI).log("no sink registered"));
}

TEST(LoggerTest, UiSinkReceivesGuiModeMessage)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    std::vector<std::string> received;
    const dnf_composer::test::ScopedUiSink guard{
        [&received](LogLevel, const std::string& message) { received.push_back(message); } };

    Logger(LogLevel::INFO, LogOutputMode::GUI).log("sink-marker");

    ASSERT_EQ(received.size(), 1u);
    EXPECT_NE(received.front().find("sink-marker"), std::string::npos);
}

TEST(LoggerTest, UiSinkReceivesAllModeMessage)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    std::vector<std::string> received;
    const dnf_composer::test::ScopedUiSink guard{
        [&received](LogLevel, const std::string& message) { received.push_back(message); } };

    Logger(LogLevel::INFO, LogOutputMode::ALL).log("all-mode-marker");

    ASSERT_EQ(received.size(), 1u);
    EXPECT_NE(received.front().find("all-mode-marker"), std::string::npos);
}

TEST(LoggerTest, UiSinkDoesNotReceiveConsoleOnlyMessage)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    std::vector<std::string> received;
    const dnf_composer::test::ScopedUiSink guard{
        [&received](LogLevel, const std::string& message) { received.push_back(message); } };

    Logger(LogLevel::INFO, LogOutputMode::CONSOLE).log("console-only-marker");

    EXPECT_TRUE(received.empty());
}

TEST(LoggerTest, UiSinkReceivesTheReportedLevel)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    LogLevel receivedLevel = LogLevel::DEBUG;
    const dnf_composer::test::ScopedUiSink guard{
        [&receivedLevel](const LogLevel level, const std::string&) { receivedLevel = level; } };

    Logger(LogLevel::WARNING, LogOutputMode::GUI).log("level-check");

    EXPECT_EQ(receivedLevel, LogLevel::WARNING);
}

TEST(LoggerTest, UiSinkClearedAfterScopeReturnsToNoop)
{
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    std::vector<std::string> received;
    {
        const dnf_composer::test::ScopedUiSink guard{
            [&received](LogLevel, const std::string& message) { received.push_back(message); } };
        Logger(LogLevel::INFO, LogOutputMode::GUI).log("inside-scope");
    }

    EXPECT_NO_THROW(Logger(LogLevel::INFO, LogOutputMode::GUI).log("after-scope"));
    ASSERT_EQ(received.size(), 1u);
    EXPECT_NE(received.front().find("inside-scope"), std::string::npos);
}

TEST(LoggerTest, UiSinkCallingSetUiSinkReentrantlyDoesNotDeadlock)
{
    // log_ui() must release logSinkMutex before invoking the sink: a sink that
    // calls setUiSink() (e.g. to unregister itself) reacquires the same
    // non-recursive mutex, which deadlocks if log_ui() is still holding it.
    const dnf_composer::test::ScopedMinLogLevel levelGuard{ LogLevel::DEBUG };
    bool sinkRan = false;
    const dnf_composer::test::ScopedUiSink guard{
        [&sinkRan](LogLevel, const std::string&)
        {
            sinkRan = true;
            Logger::setUiSink([](LogLevel, const std::string&) {});
        } };

    EXPECT_NO_THROW(Logger(LogLevel::INFO, LogOutputMode::GUI).log("reentrant-marker"));
    EXPECT_TRUE(sinkRan);
}
