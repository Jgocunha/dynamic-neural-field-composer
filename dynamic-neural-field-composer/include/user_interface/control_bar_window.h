#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "user_interface/log_window.h"


namespace dnf_composer::user_interface
{
    /// @brief Top control bar: simulation info, run/pause/stop buttons and timescale controls.
    class ControlBarWindow final : public imgui_kit::UserInterfaceWindow
    {
    private:
        std::shared_ptr<Simulation> simulation;

    public:
        /// @brief Construct the control bar for a simulation.
        /// @param simulation Simulation whose run state and timescale are shown/controlled.
        explicit ControlBarWindow(const std::shared_ptr<Simulation>& simulation);

        ControlBarWindow(const ControlBarWindow&)            = delete;
        ControlBarWindow& operator=(const ControlBarWindow&) = delete;
        ControlBarWindow(ControlBarWindow&&)                 = delete;
        ControlBarWindow& operator=(ControlBarWindow&&)      = delete;

        /// @brief Draw the control bar for this frame.
        void render() override;
        /// @brief Draw the control bar's contents (info, control buttons, timescale).
        void drawContents() const;
        ~ControlBarWindow() override = default;
    private:
        void drawSimulationInfo() const;
        void drawSimulationControlButtons() const;
        void drawTimescale() const;
        void drawRunControl() const;
    };
}
