#include "application/style.h"
#include "exceptions/exception.h"
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <fstream>
#include <unordered_map>

namespace dnf_composer
{
    static ImVec2 toVec2(const nlohmann::json& j) { return { j[0].get<float>(), j[1].get<float>() }; }
    static ImVec4 toVec4(const nlohmann::json& j) { return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>() }; }

    static const std::unordered_map<std::string, ImGuiCol> kColorIndex = {
        { "Text",                   ImGuiCol_Text },
        { "TextDisabled",           ImGuiCol_TextDisabled },
        { "WindowBg",               ImGuiCol_WindowBg },
        { "ChildBg",                ImGuiCol_ChildBg },
        { "PopupBg",                ImGuiCol_PopupBg },
        { "Border",                 ImGuiCol_Border },
        { "BorderShadow",           ImGuiCol_BorderShadow },
        { "FrameBg",                ImGuiCol_FrameBg },
        { "FrameBgHovered",         ImGuiCol_FrameBgHovered },
        { "FrameBgActive",          ImGuiCol_FrameBgActive },
        { "TitleBg",                ImGuiCol_TitleBg },
        { "TitleBgActive",          ImGuiCol_TitleBgActive },
        { "TitleBgCollapsed",       ImGuiCol_TitleBgCollapsed },
        { "MenuBarBg",              ImGuiCol_MenuBarBg },
        { "ScrollbarBg",            ImGuiCol_ScrollbarBg },
        { "ScrollbarGrab",          ImGuiCol_ScrollbarGrab },
        { "ScrollbarGrabHovered",   ImGuiCol_ScrollbarGrabHovered },
        { "ScrollbarGrabActive",    ImGuiCol_ScrollbarGrabActive },
        { "CheckMark",              ImGuiCol_CheckMark },
        { "SliderGrab",             ImGuiCol_SliderGrab },
        { "SliderGrabActive",       ImGuiCol_SliderGrabActive },
        { "Button",                 ImGuiCol_Button },
        { "ButtonHovered",          ImGuiCol_ButtonHovered },
        { "ButtonActive",           ImGuiCol_ButtonActive },
        { "Header",                 ImGuiCol_Header },
        { "HeaderHovered",          ImGuiCol_HeaderHovered },
        { "HeaderActive",           ImGuiCol_HeaderActive },
        { "Separator",              ImGuiCol_Separator },
        { "SeparatorHovered",       ImGuiCol_SeparatorHovered },
        { "SeparatorActive",        ImGuiCol_SeparatorActive },
        { "ResizeGrip",             ImGuiCol_ResizeGrip },
        { "ResizeGripHovered",      ImGuiCol_ResizeGripHovered },
        { "ResizeGripActive",       ImGuiCol_ResizeGripActive },
        { "Tab",                    ImGuiCol_Tab },
        { "TabHovered",             ImGuiCol_TabHovered },
        { "TabActive",              ImGuiCol_TabActive },
        { "TabUnfocused",           ImGuiCol_TabUnfocused },
        { "TabUnfocusedActive",     ImGuiCol_TabUnfocusedActive },
        { "PlotLines",              ImGuiCol_PlotLines },
        { "PlotLinesHovered",       ImGuiCol_PlotLinesHovered },
        { "PlotHistogram",          ImGuiCol_PlotHistogram },
        { "PlotHistogramHovered",   ImGuiCol_PlotHistogramHovered },
        { "TableHeaderBg",          ImGuiCol_TableHeaderBg },
        { "TableBorderStrong",      ImGuiCol_TableBorderStrong },
        { "TableBorderLight",       ImGuiCol_TableBorderLight },
        { "TableRowBg",             ImGuiCol_TableRowBg },
        { "TableRowBgAlt",          ImGuiCol_TableRowBgAlt },
        { "TextSelectedBg",         ImGuiCol_TextSelectedBg },
        { "DragDropTarget",         ImGuiCol_DragDropTarget },
        { "NavHighlight",           ImGuiCol_NavHighlight },
        { "NavWindowingHighlight",  ImGuiCol_NavWindowingHighlight },
        { "NavWindowingDimBg",      ImGuiCol_NavWindowingDimBg },
        { "ModalWindowDimBg",       ImGuiCol_ModalWindowDimBg },
    };

    void applyImGuiStyle(const std::string& jsonPath)
    {
        std::ifstream f(jsonPath);
        if (!f.is_open())
            throw Exception(ErrorCode::APP_INIT);

        const auto j = nlohmann::json::parse(f);
        ImGuiStyle& s = ImGui::GetStyle();

        const auto& m = j.at("metrics");
        s.Alpha                      = m.value("Alpha",               s.Alpha);
        s.DisabledAlpha              = m.value("DisabledAlpha",        s.DisabledAlpha);
        s.WindowPadding              = toVec2(m.at("WindowPadding"));
        s.WindowRounding             = m.value("WindowRounding",       s.WindowRounding);
        s.WindowBorderSize           = m.value("WindowBorderSize",     s.WindowBorderSize);
        s.WindowMinSize              = toVec2(m.at("WindowMinSize"));
        s.WindowTitleAlign           = toVec2(m.at("WindowTitleAlign"));
        s.WindowMenuButtonPosition   = static_cast<ImGuiDir>(m.value("WindowMenuButtonPosition", static_cast<int>(s.WindowMenuButtonPosition)));
        s.ChildRounding              = m.value("ChildRounding",        s.ChildRounding);
        s.ChildBorderSize            = m.value("ChildBorderSize",      s.ChildBorderSize);
        s.PopupRounding              = m.value("PopupRounding",        s.PopupRounding);
        s.PopupBorderSize            = m.value("PopupBorderSize",      s.PopupBorderSize);
        s.FramePadding               = toVec2(m.at("FramePadding"));
        s.FrameRounding              = m.value("FrameRounding",        s.FrameRounding);
        s.FrameBorderSize            = m.value("FrameBorderSize",      s.FrameBorderSize);
        s.ItemSpacing                = toVec2(m.at("ItemSpacing"));
        s.ItemInnerSpacing           = toVec2(m.at("ItemInnerSpacing"));
        s.CellPadding                = toVec2(m.at("CellPadding"));
        s.IndentSpacing              = m.value("IndentSpacing",        s.IndentSpacing);
        s.ColumnsMinSpacing          = m.value("ColumnsMinSpacing",    s.ColumnsMinSpacing);
        s.ScrollbarSize              = m.value("ScrollbarSize",        s.ScrollbarSize);
        s.ScrollbarRounding          = m.value("ScrollbarRounding",    s.ScrollbarRounding);
        s.GrabMinSize                = m.value("GrabMinSize",          s.GrabMinSize);
        s.GrabRounding               = m.value("GrabRounding",         s.GrabRounding);
        s.TabRounding                = m.value("TabRounding",          s.TabRounding);
        s.TabBorderSize              = m.value("TabBorderSize",        s.TabBorderSize);
        s.ColorButtonPosition        = static_cast<ImGuiDir>(m.value("ColorButtonPosition", static_cast<int>(s.ColorButtonPosition)));
        s.ButtonTextAlign            = toVec2(m.at("ButtonTextAlign"));
        s.SelectableTextAlign        = toVec2(m.at("SelectableTextAlign"));

        for (const auto& [name, rgba] : j.at("colors").items())
        {
            if (const auto it = kColorIndex.find(name); it != kColorIndex.end())
                s.Colors[it->second] = toVec4(rgba);
        }
    }
}
