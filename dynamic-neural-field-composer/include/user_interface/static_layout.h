#pragma once

#include <memory>
#include <chrono>

#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "user_interface/simulation_window.h"
#include "user_interface/element_window.h"
#include "user_interface/node_graph_window.h"
#include "user_interface/plots_window.h"
#include "application/application.h"
#include "tools/utils.h"
#include "user_interface/widgets.h"
#include "user_interface/log_window.h"
#include "user_interface/control_bar_window.h"
#include "user_interface/status_bar_window.h"

namespace dnf_composer::user_interface
{
	class StaticLayoutWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation>         simulation;
		std::shared_ptr<Visualization>      visualization;

		std::unique_ptr<ControlBarWindow>   controlBarWindow;
		std::unique_ptr<SimulationWindow>   simulationWindow;
		std::unique_ptr<ElementWindow>      elementWindow;
		std::unique_ptr<NodeGraphWindow>    nodeGraphWindow;
		std::unique_ptr<PlotsWindow>		plotsWindow;
		std::unique_ptr<StatusBarWindow>    statusBarWindow;
		std::unique_ptr<LogWindow>			logWindow;

	public:
		StaticLayoutWindow(const std::shared_ptr<Simulation>&    simulation,
		                   const std::shared_ptr<Visualization>&  visualization);

		StaticLayoutWindow(const StaticLayoutWindow&)            = delete;
		StaticLayoutWindow& operator=(const StaticLayoutWindow&) = delete;
		StaticLayoutWindow(StaticLayoutWindow&&)                 = delete;
		StaticLayoutWindow& operator=(StaticLayoutWindow&&)      = delete;

		void render() override;
		~StaticLayoutWindow() override = default;

	private:
		void renderWindows() const;
		void drawPanels() const;

		// void panelTopBar        (ImVec2 pos, ImVec2 size) const;
		// void renderTopBarLeft   (float ui, float btnSz) const;
		// void renderTopBarRight  (float ui, float btnSz) const;

		void drawPanelControlBar    (ImVec2 pos, ImVec2 size) const;
		void drawPanelSimulation    (ImVec2 pos, ImVec2 size) const;
		void drawPanelElement       (ImVec2 pos, ImVec2 size) const;
		void drawPanelNodeGraph     (ImVec2 pos, ImVec2 size) const;
		void drawPanelStatusBar     (ImVec2 pos, ImVec2 size) const;
	};
}
