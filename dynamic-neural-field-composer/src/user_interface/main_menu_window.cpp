#include "user_interface/main_menu_window.h"

#include "application/application.h"

namespace dnf_composer::user_interface {

    MainMenuWindow::MainMenuWindow()
    { }

    void MainMenuWindow::decorations()
    {
        pos = ImGui::GetWindowPos();
        draw = ImGui::GetWindowDrawList();


        draw->AddRectFilled(pos, ImVec2(pos.x + 805, pos.y + 480), ImColor(235, 235, 240), 12);
    }

    void MainMenuWindow::tabs_()
    {

        ImGui::BeginChild("##tabs", ImVec2(50, 475));
        ImGui::SetCursorPos(ImVec2(27, 7)); ImGui::PushFont(iconfont); ImGui::Text("K"); ImGui::PopFont();
        ImGui::SetCursorPos(ImVec2(17, 75));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 16));
        ImGui::BeginGroup();
        {
            if (widgets::tab("A", 0 == tabs)) tabs = 0;
            if (widgets::tab("B", 1 == tabs)) tabs = 1;
            if (widgets::tab("C", 2 == tabs)) tabs = 2;
            if (widgets::tab("D", 3 == tabs)) tabs = 3;
            if (widgets::tab("E", 4 == tabs)) tabs = 4;
            ImGui::SetCursorPosY(450);
            if (widgets::settingsButton("L")) sett = 1;
        }
        ImGui::EndGroup();
        ImGui::PopStyleVar();
        ImGui::EndChild();
    }

    void MainMenuWindow::subtabs_()
    {
        if (tabs == 1)
        {
            ImGui::SetCursorPos(ImVec2(50, 0));
            ImGui::BeginChild("Visuals", ImVec2(160, 475));
            {
                ImGui::SetCursorPos(ImVec2(15, 50));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
                ImGui::BeginGroup();
                {
                    ImGui::TextColored(ImColor(195, 195, 200), "ENEMY");
                    //ImGui::Spacing(0);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));
                    if (widgets::sub("ESP", 0 == subtabs)) subtabs = 0;
                    if (widgets::sub("Chams", 1 == subtabs)) subtabs = 1;
                    if (widgets::sub("Other", 2 == subtabs)) subtabs = 2;
                    //ImGui::Spacing(7);
                    ImGui::Dummy(ImVec2(0.0f, 7.0f));

                    ImGui::TextColored(ImColor(195, 195, 200), "TEAM");
                    ImGui::Spacing();
                     if (widgets::sub("ESP##1", 3 == subtabs)) subtabs = 3;
                     if (widgets::sub("Chams##1", 4 == subtabs)) subtabs = 4;
                     if (widgets::sub("Other##1", 5 == subtabs)) subtabs = 5;

                   // ImGui::Spacing(7);
                    ImGui::Dummy(ImVec2(0.0f,7.0f));

                    ImGui::TextColored(ImColor(195, 195, 200), "WORLD");
                    ImGui::Spacing();
                    if (widgets::sub("ESP##2", 6 == subtabs)) subtabs = 6;
                    if (widgets::sub("Chams##2", 7 == subtabs)) subtabs = 7;
                    if (widgets::sub("Other##2", 8 == subtabs)) subtabs = 8;
                }
                ImGui::EndGroup();
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
        }

    }

    void MainMenuWindow::function()
    {

        if (tabs == 1)
        {
            if (subtabs == 0)
            {
                ImGui::SetCursorPos(ImVec2(210, 0));
                ImGui::BeginChild("General", ImVec2(280, 475));
                {
                    ImGui::SetCursorPos(ImVec2(15, 50));
                    ImGui::BeginGroup();
                    {
                        widgets::checkbox("Checkbox", &checkbox);
                        ImGui::SliderInt("SliderInt", &sliderint, 0, 100);
                        ImGui::Button("Button", ImVec2(220, 30));
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
                ImGui::SameLine(0, 0);
                ImGui::BeginChild("Preview", ImVec2(315, 475));
                {

                }
                ImGui::EndChild();
            }
        }

    }


    void MainMenuWindow::render() {
        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar);
        {
            static int x = 805 * dpi_scale, y = 575 * dpi_scale;
            pos = ImGui::GetWindowPos();
            draw = ImGui::GetWindowDrawList();

            ImGui::SetWindowSize(ImVec2(ImFloor(x * dpi_scale), ImFloor(y * dpi_scale)));

            decorations();
            tabs_();
            subtabs_();
            function();

        }
        ImGui::End();
    }

    void MainMenuWindow::renderTabs() {
        ImGui::SetCursorPos(ImVec2(15, 15)); // Add padding at the top-left
        ImGui::BeginChild("Tabs", ImVec2(80, 400), true, ImGuiWindowFlags_NoScrollbar);

        if (ImGui::Button("A", ImVec2(60, 40))) currentTab = 0; // Adjust button size
        ImGui::Spacing(); // Add spacing between buttons
        if (ImGui::Button("B", ImVec2(60, 40))) currentTab = 1;
        ImGui::Spacing();
        if (ImGui::Button("C", ImVec2(60, 40))) currentTab = 2;

        ImGui::EndChild();

    }

    void MainMenuWindow::renderSubTabs() {
        if (currentTab == 1) {
            ImGui::BeginChild("SubTabs", ImVec2(120, 400), true);

            if (ImGui::Selectable("Sub 1", currentSubTab == 0, ImGuiSelectableFlags_AllowDoubleClick)) currentSubTab = 0;
            ImGui::Spacing();
            if (ImGui::Selectable("Sub 2", currentSubTab == 1, ImGuiSelectableFlags_AllowDoubleClick)) currentSubTab = 1;

            ImGui::EndChild();
        }

    }

    void MainMenuWindow::renderContent() {
        ImGui::SetCursorPos(ImVec2(200, 15)); // Adjust content alignment
        ImGui::BeginChild("Content", ImVec2(400, 400), true);

        if (currentTab == 0) {
            ImGui::Text("Tab A Content");
        } else if (currentTab == 1) {
            if (currentSubTab == 0) {
                ImGui::Text("SubTab 1 Content");
                ImGui::Checkbox("Custom Checkbox", &checkBoxState);
            } else if (currentSubTab == 1) {
                ImGui::Text("SubTab 2 Content");
                ImGui::SliderFloat("Slider", &sliderValue, 0, 100);
            }
        }

        ImGui::EndChild();

    }
}
