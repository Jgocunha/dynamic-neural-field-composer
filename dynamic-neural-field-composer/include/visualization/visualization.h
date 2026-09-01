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
	/// @brief Fill @p data and @p legends with one entry per plot data source.
	///
	/// Passing the same buffers back on the next frame reuses their storage,
	/// which is why Visualization::render() keeps them as members instead of
	/// constructing fresh vectors inside its per-plot loop every frame (#53).
	/// @p data is cleared and refilled: raw pointers own no storage of their
	/// own, so this is already allocation-free at steady state. @p legends is
	/// resized (not cleared) and each surviving @c std::string is overwritten
	/// in place, so a long label's own character-buffer allocation is reused
	/// rather than freed and rebuilt every call -- @c clear() on the vector
	/// would destroy each @c std::string and lose exactly that storage.
	///
	/// The component pointers are re-read from @p simulation on every call and
	/// deliberately not cached across calls -- a component vector reallocates
	/// when its element is resized, so a retained pointer would dangle.
	///
	/// Every source must exist in @p simulation; callers check this first (via
	/// @c Simulation::componentExists) and drop the plot otherwise.
	///
	/// It is deliberately pure and free-standing so the buffer contract can be
	/// tested without an ImGui context, the same extraction rationale as
	/// @c resolveManualHeatmapDimensions in heatmap.h.
	///
	/// @param simulation  Simulation to read the component vectors from.
	/// @param sources     {element-name, component-name} pairs to gather.
	/// @param data        Out: pointer to each source's component vector.
	/// @param legends     Out: "element - component" label for each source.
	void gatherPlotSeries(const Simulation& simulation,
		const std::vector<std::pair<std::string, std::string>>& sources,
		std::vector<std::vector<double>*>& data,
		std::vector<std::string>& legends);

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

		// Scratch buffers for the render path, reused across frames so that a
		// steady-state frame allocates nothing (#53). Refilled per plot by
		// gatherPlotSeries(); never read outside a single render()/renderTile()
		// call, and never holding component pointers between calls.
		std::vector<std::vector<double>*> renderDataBuffer;
		std::vector<std::string> renderLegendBuffer;
		std::vector<int> plotsToRemoveBuffer;
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
		/// @param parameters        Common plot parameters (type, dimensions, annotations).
		/// @param specificParameters  Type-specific parameters (e.g. LinePlotParameters).
		/// @param name              Element unique name.
		/// @param component         Component name (e.g. "activation", "output").
		void plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::string& name, const std::string& component);

		/// @brief Add a data source to an existing plot identified by @p plotId.
		/// @param plotId  Unique plot identifier.
		/// @param data    List of {element-name, component-name} pairs to add.
		void plot(int plotId, const std::vector<std::pair<std::string, std::string>>& data);

		/// @brief Add a single data source to an existing plot.
		/// @param plotId    Unique plot identifier.
		/// @param name      Element unique name.
		/// @param component Component name (e.g. "activation", "output").
		void plot(int plotId, const std::string& name, const std::string& component);

		/// @brief Remove the plot with the given @p plotId.
		/// @param plotId Unique plot identifier.
		void removePlot(int plotId);

		/// @brief Remove all registered plots.
		void removeAllPlots();

		/// @brief Remove a specific data source from a plot.
		/// @param plotId  Target plot.
		/// @param data    The {element-name, component-name} pair to remove.
		void removePlottingDataFromPlot(int plotId, const std::pair<std::string, std::string>& data);

		/// @brief Return the underlying simulation.
		/// @return The simulation this visualization plots data from.
		[[nodiscard]] std::shared_ptr<Simulation> getSimulation() const { return simulation; }

		/// @brief Return the full plots map (plot → data sources).
		/// @return Map from each registered plot to its {element-name, component-name} data sources.
		std::unordered_map<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>> getPlots() { return plots; }

		/// @brief Render all plots (creates ImGui windows or child regions as appropriate).
		void render();

		/// @brief Render a single plot's content without creating an ImGui window (caller owns the region).
		/// @param plotId  Unique plot identifier.
		void renderTile(int plotId);

		/// @brief Set a suffix appended to all ImGui window IDs to avoid collisions.
		/// @param s Suffix to append to plot window IDs.
		void setWindowIdSuffix(const std::string& s) { windowSuffix = s; }
		/// @brief Clear the ImGui window ID suffix set by setWindowIdSuffix().
		void clearWindowIdSuffix() { windowSuffix.clear(); }
	};
}
