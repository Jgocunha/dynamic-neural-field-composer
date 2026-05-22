
#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#include <imgui-platform-kit/user_interface_window.h>

#include "application/application.h"
#include "user_interface/fonts/IconsFontAwesome6.h"

extern ImFont* g_BlackLargeFont;
extern ImFont* g_MonoMediumFont;
extern ImFont* g_MediumIconsFont;

namespace dnf_composer::user_interface
{
    struct LogEntry
	{
        std::string message;
        ImVec4 color;
    };

    class LogWindow final : public imgui_kit::UserInterfaceWindow
	{
    private:
        inline static std::vector<LogEntry> logs;
        inline static ImGuiTextFilter filter;
        inline static bool autoScroll = true;
        inline static bool isWindowActive = false;
        inline static bool s_expanded = false;

    public:
        LogWindow();
        static void addLog(const ImVec4& color, const char* fmt, ...) IM_FMTARGS(2);
        void render() override { draw(); }
        static bool isActive()            { return isWindowActive; }
        static void setActive(bool v)     { isWindowActive = v; }
        static void setExpanded(bool v)   { s_expanded = v; }
        ~LogWindow() override = default;
    private:
        static void clean() { logs.clear(); }
        static void draw();
        static void renderContent();
    };
}
