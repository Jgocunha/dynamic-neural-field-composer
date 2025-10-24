#pragma once

#include <imgui-platform-kit/user_interface_window.h>
#include "widgets.h"
#include <imgui-platform-kit/user_interface.h>

#include "user_interface/simulation_window.h"

namespace dnf_composer::user_interface
{
    static bool BeginCard(const char* id, const ImVec2& top_left, const ImVec2& size,
                      const char* title, float ui)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 a = top_left;
        ImVec2 b = ImVec2(top_left.x + size.x, top_left.y + size.y);

        const float r   = 10.0f * ui;
        const float pad = 12.0f * ui;
        const float th  = 32.0f * ui;     // title bar height

        // shadow
        // dl->AddRectFilled(a + ImVec2(0, 4*ui), b + ImVec2(0, 4*ui),
        //                   ImColor(0,0,0,28), r);
        // background
        dl->AddRectFilled(a, b, ImColor(255,255,255, 235), r);
        // border
        dl->AddRect(a, b, ImColor(0,0,0,32), r);

        // title bar (very subtle)
        ImVec2 tb_a = a;
        ImVec2 tb_b = ImVec2(b.x, a.y + th);
        dl->AddRectFilled(tb_a, tb_b, ImColor(245,247,250, 235), r, ImDrawFlags_RoundCornersTop);
        dl->AddLine(ImVec2(tb_a.x, tb_b.y), ImVec2(tb_b.x, tb_b.y), ImColor(0,0,0,22), 1.0f);

        // title text
        ImVec2 title_pos = ImVec2(a.x + pad, a.y + (th - ImGui::GetTextLineHeight())*0.5f);
        ImU32 title_col  = ImColor(80, 80, 80, 255);
        dl->AddText(title_pos, title_col, title);

        // set cursor to the card’s inner body area and start a child
        ImVec2 body_pos  = ImVec2(a.x + pad, a.y + th + pad);
        ImVec2 body_size = ImVec2(size.x - 2*pad, size.y - th - 2*pad);

        ImGui::SetCursorScreenPos(body_pos);
        return ImGui::BeginChild(id, body_size, false,
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    }

    static void EndCard()
    {
        ImGui::EndChild();
    }

    class MainMenuWindow : public imgui_kit::UserInterfaceWindow
    {
        std::shared_ptr<Simulation> simulation;
        int tab;
        std::unique_ptr<dnf_composer::user_interface::SimulationWindow> sim_window_;
        ImVec2 rMainMin;
        ImVec2 rMainMax;
        float ui;
    public:
        MainMenuWindow(const std::shared_ptr<Simulation>& simulation);        // Constructor to initialize the window properties
        void render() override;  // Override the render method
    private:
        void tabs();
        // void file();
        // void edit();
        // void simulation();
        // void tools();
        // void help();
        void build_tab();
    };

}