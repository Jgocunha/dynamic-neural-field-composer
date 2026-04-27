#pragma once

#include <iostream>
#include <vector>

#include "simulation/simulation.h"
#include "exceptions/exception.h"
#include "plot.h"
#include "tools/logger.h"
#include "visualization/lineplot.h"
#include "visualization/heatmap.h"

/// @defgroup visualization Visualization
/// @brief Plot management and rendering for simulation data.

namespace dnf_composer
{
	/// @brief Manages a collection of plots driven by a running Simulation.
	///
	/// Visualization owns a set of Plot instances. Each plot is associated with
	/// one or more (element-name, component-name) data sources. On every call to
	/// @c render() the visualization pulls current data from the simulation and
	/// forwards it to each plot's renderer.
	///
	/// @ingroup visualization
	class Visualization
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::unordered_map<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>> plots;
		std::string windowSuffix;
	public:
		/// @brief Construct a Visualization backed by the given simulation.
		/// @param simulation  The simulation whose data will be visualized.
		explicit Visualization(const std::shared_ptr<Simulation>& simulation);

		/// @brief Add a blank plot of the given type.
		/// @param type  Plot type (default LINE_PLOT).
		void plot(PlotType type = PlotType::LINE_PLOT);

		/// @brief Add a new line plot with the given (element, component) data sources.
		/// @param data  List of {element-name, component-name} pairs to plot.
		void plot(const std::vector<std::pair<std::string, std::string>>& data);

		/// @brief Add a new line plot with a single data source.
		/// @param name       Element unique name.
		/// @param component  Component name (e.g. "activation", "output").
		void plot(const std::string& name, const std::string& component);

		/// @brief Add a fully configured plot.
		/// @param parameters        Common plot parameters (type, dimensions, annotations).
		/// @param specificParameters  Type-specific parameters (e.g. LinePlotParameters).
		/// @param data              List of {element-name, component-name} pairs.
		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::vector<std::pair<std::string, std::string>>& data);

		/// @brief Add a fully configured plot with a single data source.
		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::string& name, const std::string& component);

		/// @brief Add a data source to an existing plot identified by @p plotId.
		/// @param plotId  Unique plot identifier.
		/// @param data    List of {element-name, component-name} pairs to add.
		void plot(int plotId, const std::vector<std::pair<std::string, std::string>>& data);

		/// @brief Add a single data source to an existing plot.
		void plot(int plotId, const std::string& name, const std::string& component);

		/// @brief Remove the plot with the given @p plotId.
		void removePlot(int plotId);

		/// @brief Remove all registered plots.
		void removeAllPlots();

		/// @brief Remove a specific data source from a plot.
		/// @param plotId  Target plot.
		/// @param data    The {element-name, component-name} pair to remove.
		void removePlottingDataFromPlot(int plotId, const std::pair<std::string, std::string>& data);

		/// @brief Return the underlying simulation.
		[[nodiscard]] std::shared_ptr<Simulation> getSimulation() const { return simulation; }

		/// @brief Return the full plots map (plot → data sources).
		std::unordered_map<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>> getPlots() { return plots; }

		/// @brief Render all plots (creates ImGui windows or child regions as appropriate).
		void render();

		/// @brief Render a single plot's content without creating an ImGui window (caller owns the region).
		/// @param plotId  Unique plot identifier.
		void renderTile(int plotId);

		/// @brief Set a suffix appended to all ImGui window IDs to avoid collisions.
		void setWindowIdSuffix(const std::string& s) { windowSuffix = s; }
		void clearWindowIdSuffix() { windowSuffix.clear(); }
	};
}
