#pragma once

#include <memory>
#include <imgui-platform-kit/user_interface_window.h>

#include "simulation/simulation.h"
#include "visualization/visualization.h"
#include "user_interface/simulation_window.h"
#include "user_interface/element_window.h"
#include "user_interface/node_graph_window.h"
#include "user_interface/field_metrics_window.h"
#include "user_interface/plot_control_window.h"
#include "user_interface/plots_window.h"
#include "user_interface/log_window.h"

namespace dnf_composer::user_interface
{
	class StaticLayoutWindow final : public imgui_kit::UserInterfaceWindow
	{
	private:
		std::shared_ptr<Simulation>         simulation;
		std::shared_ptr<Visualization>      visualization;

		std::unique_ptr<SimulationWindow>   simulationWindow;
		std::unique_ptr<ElementWindow>      elementWindow;
		std::unique_ptr<NodeGraphWindow>    nodeGraphWindow;
		std::unique_ptr<FieldMetricsWindow> fieldMetricsWindow;
		std::unique_ptr<PlotControlWindow>  plotControlWindow;
		std::unique_ptr<PlotsWindow>        plotsWindow;

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
		void drawPanels();


		void panelSimulation  (ImVec2 pos, ImVec2 size) const;
		void panelFieldMonitor(ImVec2 pos, ImVec2 size) const;
		void panelElement     (ImVec2 pos, ImVec2 size) const;
		void panelNodeGraph   (ImVec2 pos, ImVec2 size) const;
		void panelPlots       (ImVec2 pos, ImVec2 size);
		void panelLogs        (ImVec2 pos, ImVec2 size) const;
		void panelPlotControl (ImVec2 pos, ImVec2 size) const;
	};
}
