#include "user_interface/control_bar_window.h"

#include <chrono>
#include "user_interface/fonts/IconsFontAwesome6.h"


namespace dnf_composer
{
    extern ImFont* g_BlackLargeFont;
    extern ImFont* g_BlackMediumFont;
    extern ImFont* g_MonoMediumFont;
    extern ImFont* g_LargeIconsFont;

    namespace user_interface
    {
        ControlBarWindow::ControlBarWindow(const std::shared_ptr<Simulation> &simulation)
            :simulation(simulation)
        {}

        void ControlBarWindow::render()
        {
            const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoScrollWithMouse;

            ImGui::PushFont(g_BlackLargeFont);
            const bool open = ImGui::Begin("Control", nullptr, flags);
            ImGui::PopFont();

            if (open)
                drawContents();

            ImGui::End();
        }

        void ControlBarWindow::drawContents() const
        {
            const float ui = ImGui::GetIO().FontGlobalScale;

            if (const float slackTop = (ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight()) * 0.5f; slackTop > 0.0f)
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + slackTop);

            drawSimulationInfo();
            ImGui::SameLine(0, 20.0f * ui);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine(0, 20.0f * ui);

            drawSimulationControlButtons();
            ImGui::SameLine(0, 20.0f * ui);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine(0, 20.0f * ui);

            drawTimescale();
            ImGui::SameLine(0, 20.0f * ui);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine(0, 20.0f * ui);

            drawRunControl();
            drawConsoleButton();
        }

        void ControlBarWindow::drawSimulationInfo() const
        {
            const std::string simulationId = simulation->getUniqueIdentifier();

            ImGui::AlignTextToFramePadding();
            ImGui::PushFont(g_BlackMediumFont);
            ImGui::TextUnformatted(simulationId.c_str());
            ImGui::SameLine(0, 2);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(".dnf");
            ImGui::PopFont();
        }

        void ControlBarWindow::drawSimulationControlButtons() const
        {
            const float ui    = ImGui::GetIO().FontGlobalScale;
            const float btnSz = ImGui::GetFrameHeight();
            const ImVec2 bSz(btnSz, btnSz);

            constexpr auto kBg     = ImVec4(0.96f, 0.98f, 0.00f, 0.0f);
            constexpr auto kHover  = ImVec4(0.90f, 0.97f, 0.94f, 1.0f);
            constexpr auto kActive = ImVec4(0.85f, 0.96f, 0.92f, 1.0f);
            constexpr auto kRed   = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button,        kBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kActive);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  6.0f * ui);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,   ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushFont(g_LargeIconsFont);

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
            if (ImGui::Button(ICON_FA_PLAY "##cb_play",   bSz)) { simulation->init(); simulation->resume(); }
            ImGui::PopStyleColor();
            const bool hPlay   = ImGui::IsItemHovered();

            ImGui::SameLine(0, 8);
            if (ImGui::Button(ICON_FA_PAUSE "##cb_pause",  bSz)) simulation->pause();
            const bool hPause  = ImGui::IsItemHovered();

            ImGui::SameLine(0, 8);
            if (ImGui::Button(ICON_FA_FORWARD_FAST "##cb_resume", bSz)) simulation->resume();
            const bool hResume = ImGui::IsItemHovered();

            ImGui::SameLine(0, 8);
            ImGui::PushStyleColor(ImGuiCol_Text, kRed);
            if (ImGui::Button(ICON_FA_STOP "##cb_stop",   bSz)) simulation->close();
            ImGui::PopStyleColor();
            const bool hStop   = ImGui::IsItemHovered();

            ImGui::PopFont();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (hPlay)   ImGui::SetTooltip("Play");
            if (hPause)  ImGui::SetTooltip("Pause");
            if (hResume) ImGui::SetTooltip("Resume");
            if (hStop)   ImGui::SetTooltip("Stop");
        }

        void ControlBarWindow::drawTimescale() const
        {
            const float ui = ImGui::GetIO().FontGlobalScale;

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("\xce\x94t");
            ImGui::SameLine(0, 6);

            static double dt = simulation->getDeltaT();
            constexpr double kMin = 0.01, kMax = 100.0;
            ImGui::SetNextItemWidth(220.0f * ui);
            ImGui::PushFont(g_MonoMediumFont);
            if (ImGui::SliderScalar("##cb_dt", ImGuiDataType_Double, &dt, &kMin, &kMax, "%.2f"))
            {
                if (std::isfinite(dt) && dt > 0.0)
                    simulation->setDeltaT(dt);
            }
            ImGui::PopFont();
        }

        void ControlBarWindow::drawRunControl() const
        {
            const float ui = ImGui::GetIO().FontGlobalScale;

            static bool  runActive = false;
            static int   unitIdx   = 0;
            static float runValue  = 1000.0f;
            static int   startTick = 0;
            static std::chrono::steady_clock::time_point runStart;

            if (runActive)
            {
                if (unitIdx == 1)
                {
                    if (static_cast<int>(simulation->getT()) >= startTick + static_cast<int>(runValue))
                    { simulation->pause(); runActive = false; }
                }
                else
                {
                    const float elapsed = std::chrono::duration<float, std::milli>(
                        std::chrono::steady_clock::now() - runStart).count();
                    if (elapsed >= runValue)
                    { simulation->pause(); runActive = false; }
                }
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Run");
            ImGui::SameLine(0, 8);

            ImGui::PushFont(g_MonoMediumFont);
            ImGui::SetNextItemWidth(70.0f * ui);
            ImGui::InputFloat("##cb_run_val", &runValue, 0.0f, 0.0f, "%.0f");
            if (runValue < 1.0f) runValue = 1.0f;
            ImGui::PopFont();
            ImGui::SameLine(0, 4);

            constexpr const char* kUnits[] = { "ms", "ticks" };
            ImGui::SetNextItemWidth(85.0f * ui);
            ImGui::Combo("##cb_unit", &unitIdx, kUnits, 2);
            ImGui::SameLine(0, 6);

            const float btnSz = ImGui::GetFrameHeight();
            constexpr auto kBg     = ImVec4(0.96f, 0.98f, 0.00f, 0.0f);
            constexpr auto kHover  = ImVec4(0.90f, 0.97f, 0.94f, 1.0f);
            constexpr auto kActive = ImVec4(0.85f, 0.96f, 0.92f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button,        kBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  kActive);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f * ui);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushFont(g_LargeIconsFont);

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight));
            if (ImGui::Button(ICON_FA_PLAY "##cb_run", ImVec2(btnSz, btnSz)))
            {
                if (!simulation->isInitialized()) simulation->init();
                simulation->resume();
                runActive = true;
                startTick = static_cast<int>(simulation->getT());
                runStart  = std::chrono::steady_clock::now();
            }

            ImGui::PopFont();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(5);
        }

        void ControlBarWindow::drawConsoleButton()
        {
            const float  ui   = ImGui::GetIO().FontGlobalScale;
            const bool   open = LogWindow::isActive();

            // Right-align: jump to the window right edge minus group width
            const float iconW    = g_MediumIconsFont->LegacySize * ui;
            const float textW    = ImGui::CalcTextSize("Console").x;
            const float chevronW = g_SmallIconsFont->LegacySize * ui;
            const float groupW   = iconW + 6.0f * ui + textW + 6.0f * ui + chevronW;
            ImGui::SameLine(ImGui::GetWindowWidth() - groupW - ImGui::GetStyle().WindowPadding.x);

            const ImVec4 color = open
                ? ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight)
                : ImGui::GetStyleColorVec4(ImGuiCol_Text);

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::BeginGroup();

            ImGui::PushFont(g_MediumIconsFont);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(ICON_FA_TERMINAL);
            ImGui::PopFont();
            ImGui::SameLine(0, 6.0f * ui);

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Console");
            ImGui::SameLine(0, 6.0f * ui);

            ImGui::PushFont(g_SmallIconsFont);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(open ? ICON_FA_ANGLE_UP : ICON_FA_ANGLE_DOWN);
            ImGui::PopFont();

            ImGui::EndGroup();
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsItemClicked())
                LogWindow::setActive(!open);
        }
    }
}
