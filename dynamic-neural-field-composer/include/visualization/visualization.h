#pragma once

#include <iostream>
#include <vector>

#include "simulation/simulation.h"
#include "exceptions/exception.h"
#include "plot.h"
#include "tools/logger.h"
#include "visualization/lineplot.h"
#include "visualization/heatmap.h"

namespace dnf_composer
{
	class Visualization
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::unordered_map<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>> plots;
		bool               hasDockTarget = false;
		ImGuiID            dockTargetId  = 0;
		ImGuiWindowClass   dockClass{};
	public:
		explicit Visualization(const std::shared_ptr<Simulation>& simulation);

		void plot(PlotType type = PlotType::LINE_PLOT);

		void plot(const std::vector<std::pair<std::string, std::string>>& data);
		void plot(const std::string& name, const std::string& component);

		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::vector<std::pair<std::string, std::string>>& data);
		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::string& name, const std::string& component);

		void plot(int plotId, const std::vector<std::pair<std::string, std::string>>& data);
		void plot(int plotId, const std::string& name, const std::string& component);

		void removePlot(int plotId);
		void removeAllPlots();
		void removePlottingDataFromPlot(int plotId, const std::pair<std::string, std::string>& data);

		std::shared_ptr<Simulation> getSimulation() const { return simulation; }
		std::unordered_map<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>> getPlots() { return plots; }

		void render();

		// Constrain plot windows to a specific dock space (inside the Plots card)
		void setDockTarget(ImGuiID dock_id, const ImGuiWindowClass& cls) {
			dockTargetId   = dock_id;
			dockClass      = cls;
			hasDockTarget  = true;
		}
		void clearDockTarget() { hasDockTarget = false; dockTargetId = 0; }
	};
}

