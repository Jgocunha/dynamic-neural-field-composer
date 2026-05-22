#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "user_interface/log_window.h"


namespace dnf_composer::user_interface
{
    class ControlBarWindow final : public imgui_kit::UserInterfaceWindow
    {
    private:
        std::shared_ptr<Simulation> simulation;

    public:
        explicit ControlBarWindow(const std::shared_ptr<Simulation>& simulation);

        ControlBarWindow(const ControlBarWindow&)            = delete;
        ControlBarWindow& operator=(const ControlBarWindow&) = delete;
        ControlBarWindow(ControlBarWindow&&)                 = delete;
        ControlBarWindow& operator=(ControlBarWindow&&)      = delete;

        void render() override;
        void drawContents() const;
        ~ControlBarWindow() override = default;
    private:
        void drawSimulationInfo() const;
        void drawSimulationControlButtons() const;
        void drawTimescale() const;
        void drawRunControl() const;
        void drawConsoleButton() const;
    };
}
