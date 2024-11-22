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
		std::vector<std::shared_ptr<Plot>> plots;
	public:
		Visualization(const std::shared_ptr<Simulation>& simulation);

		void plot(const std::vector<std::pair<std::string, std::string>>& data);
		void plot(const std::string& name, const std::string& component);

		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::vector<std::pair<std::string, std::string>>& data);
		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::string& name, const std::string& component);

		void plot(int plotId, const std::vector<std::pair<std::string, std::string>>& data);
		void plot(int plotId, const std::string& name, const std::string& component);

		//void changePlotParameters(int plotId, const PlotParameters& parameters);
		void removePlot(int plotId);
		void removeAllPlots();
		void removePlottingDataFromPlot(int plotId, const std::pair<std::string, std::string>& data);

		std::shared_ptr<Simulation> getSimulation() const { return simulation; }
		std::vector<std::shared_ptr<Plot>> getPlots() { return plots; }
	};
}

