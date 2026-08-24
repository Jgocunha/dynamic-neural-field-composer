#pragma once

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"


namespace dnf_composer::user_interface
{
    /// @brief Bottom status bar showing the simulation's current run state (stopped/running/paused).
    class StatusBarWindow final : public imgui_kit::UserInterfaceWindow
    {
    private:
        std::shared_ptr<Simulation> simulation;

    public:
        /// @brief Construct the status bar for a simulation.
        /// @param simulation Simulation whose run state is shown.
        explicit StatusBarWindow(const std::shared_ptr<Simulation>& simulation);

        StatusBarWindow(const StatusBarWindow&)            = delete;
        StatusBarWindow& operator=(const StatusBarWindow&) = delete;
        StatusBarWindow(StatusBarWindow&&)                 = delete;
        StatusBarWindow& operator=(StatusBarWindow&&)      = delete;

        /// @brief Draw the status bar for this frame.
        void render() override;
        /// @brief Draw the status bar's contents (state indicator dot and label).
        void drawContents() const;
        ~StatusBarWindow() override = default;
    private:
    };
}
