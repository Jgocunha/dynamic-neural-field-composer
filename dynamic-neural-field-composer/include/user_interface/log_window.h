
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
    struct LogEntry
	{
        std::string message;
        ImVec4 color;
        bool resolveColorFromLevel = false;
        tools::logger::LogLevel level = tools::logger::LogLevel::INFO;
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
        static void addLog(const ImVec4& color, const char* fmt, ...) IM_FMTARGS(2);
        // Off-UI-thread-safe: unlike addLog(), performs no ImGui calls. The
        // level is stored and resolved to a color in renderContent() on the
        // UI thread instead (issue #123 sink callback).
        static void addLog(tools::logger::LogLevel level, const char* fmt, ...) IM_FMTARGS(2);
        void render() override { draw(); }
        static bool isActive()            { return isWindowActive; }
        static void setActive(bool v)     { isWindowActive = v; }
        static void setExpanded(bool v)   { s_expanded = v; }
        ~LogWindow() override = default;
    private:
        static void clean() { std::lock_guard lock(logsMutex); logs.clear(); }
        static void draw();
        static void renderContent();
    };
}
