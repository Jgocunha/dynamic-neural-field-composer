
#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#include <mutex>
#include <imgui-platform-kit/user_interface_window.h>

#include "application/application.h"
#include "tools/logger.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_MonoMediumFont;

namespace dnf_composer::user_interface
{
    /// @brief One line stored by the log window.
    struct LogEntry
	{
        std::string message; ///< Formatted log text.
        ImVec4 color; ///< Text color to draw @c message with.
        bool resolveColorFromLevel = false; ///< If true, @c color is re-derived from @c level at draw time instead of used as-is.
        tools::logger::LogLevel level = tools::logger::LogLevel::INFO; ///< Severity the entry was logged at.
    };

    /// @brief Map a logger severity to the ImGui text color used in the log window.
    ///
    /// Color is a rendering concern, so it lives here rather than in tools/logger —
    /// the logger only ever hands the UI a @c LogLevel; the UI decides how to
    /// draw it (issue #123).
    /// @param level Severity to look up.
    /// @return An accent color for DEBUG/INFO/WARNING/ERROR/FATAL; for any other
    ///         value, the current ImGui text color if a context exists, otherwise gray.
    ImVec4 getLogLevelColorCodeGui(tools::logger::LogLevel level);

    /// @brief Console-style window listing accumulated log entries, with filtering and auto-scroll.
    class LogWindow final : public imgui_kit::UserInterfaceWindow
	{
    private:
        inline static std::vector<LogEntry> logs;
        inline static std::mutex logsMutex;
        inline static ImGuiTextFilter filter;
        inline static bool autoScroll = true;
        inline static bool isWindowActive = false;
        inline static bool s_expanded = false;

    public:
        LogWindow();
        /// @brief Append a formatted log entry with an explicit color.
        /// @param color Text color to draw the entry with.
        /// @param fmt   printf-style format string.
        static void addLog(const ImVec4& color, const char* fmt, ...) IM_FMTARGS(2);
        // Off-UI-thread-safe: unlike addLog(), performs no ImGui calls. The
        // level is stored and resolved to a color in renderContent() on the
        // UI thread instead (issue #123 sink callback).
        /// @brief Append a formatted log entry whose color is resolved from @p level when drawn.
        /// @param level Severity the entry was logged at.
        /// @param fmt   printf-style format string.
        static void addLog(tools::logger::LogLevel level, const char* fmt, ...) IM_FMTARGS(2);
        /// @brief Draw the log window for this frame.
        void render() override { draw(); }
        /// @brief Check whether the log window is currently open.
        /// @return True if the log window is active.
        static bool isActive()            { return isWindowActive; }
        /// @brief Open or close the log window.
        /// @param v True to open the window, false to close it.
        static void setActive(bool v)     { isWindowActive = v; }
        /// @brief Set whether the log window is drawn in its expanded layout.
        /// @param v True for the expanded layout, false for the compact one.
        static void setExpanded(bool v)   { s_expanded = v; }
        ~LogWindow() override = default;
    private:
        static void clean() { std::lock_guard lock(logsMutex); logs.clear(); }
        static void draw();
        static void renderContent();
    };
}
