#include "user_interface/status_bar_window.h"


namespace dnf_composer {
    extern ImFont* g_BlackLargeFont;
    extern ImFont* g_BlackMediumFont;
}



namespace dnf_composer::user_interface
{
    StatusBarWindow::StatusBarWindow(const std::shared_ptr<Simulation> &simulation)
        :simulation(simulation)
    {}

    void StatusBarWindow::render()
    {
        const ImGuiWindowFlags flags = imgui_kit::getGlobalWindowFlags()
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::PushFont(g_BlackLargeFont);
        const bool open = ImGui::Begin("Status", nullptr, flags);
        ImGui::PopFont();

        if (open)
        {
            drawContents();
        }

        ImGui::End();
    }

    void StatusBarWindow::drawContents() const
    {

    }

}
