#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"


namespace dnf_composer::user_interface
{
    class StatusBarWindow final : public imgui_kit::UserInterfaceWindow
    {
    private:
        std::shared_ptr<Simulation> simulation;

    public:
        explicit StatusBarWindow(const std::shared_ptr<Simulation>& simulation);

        StatusBarWindow(const StatusBarWindow&)            = delete;
        StatusBarWindow& operator=(const StatusBarWindow&) = delete;
        StatusBarWindow(StatusBarWindow&&)                 = delete;
        StatusBarWindow& operator=(StatusBarWindow&&)      = delete;

        void render() override;
        void drawContents() const;
        ~StatusBarWindow() override = default;
    private:
    };
}
