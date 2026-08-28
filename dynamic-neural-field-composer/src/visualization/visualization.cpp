#include "visualization/visualization.h"

#include <format>

// ImGui used directly below (SetNextWindowPos/Size, GetMainViewport, ...).
// Previously reached transitively through tools/logger.h -> imgui-platform-kit/
// log_window.h; that path is gone now that tools/ no longer depends on the GUI
// stack (#123), so this file pulls in what it actually uses, same as
// lineplot.cpp already does.
#include "application/application.h"

// application.h drags in <Windows.h> (via imgui-platform-kit), which #defines
// the ERROR macro -- it shadows LogLevel::ERROR used below. tools/logger.h
// undefines it too, but only for includes already processed by the time it
// runs; application.h is included after it here, so redo the undef locally
// (same guard as logger.h).
#ifdef ERROR
#undef ERROR
#endif

namespace dnf_composer
{
	Visualization::Visualization(const std::shared_ptr<Simulation>& simulation)
	{
		if (simulation == nullptr) {
			throw Exception(ErrorCode::VIS_INVALID_SIM);
}

		this->simulation = simulation;
		plots = {};
		windowSuffix = {};
		log(tools::logger::LogLevel::INFO, "Visualization object created.");
	}

	void Visualization::plot(const PlotType type)
	{
		switch (type)
		{
			case PlotType::LINE_PLOT:
				plots[std::make_shared<LinePlot>()] = {};
				break;
			case PlotType::HEATMAP:
				plots[std::make_shared<Heatmap>()] = {};
				break;
		}
		log(tools::logger::LogLevel::INFO, std::format("Plot {} added to visualization.", plots.size() - 1));
	}

	void Visualization::plot(const std::vector<std::pair<std::string, std::string>>& data)
	{
		plots[std::make_shared<LinePlot>()] = data;
		log(tools::logger::LogLevel::INFO, std::format("Plot {} added to visualization.", plots.size() - 1));
	}

	void Visualization::plot(const std::string& name, const std::string& component)
	{
		const std::vector<std::pair<std::string, std::string>> data = { {name, component} };
		plot(data);
	}

	void Visualization::plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::vector<std::pair<std::string, std::string>>& data)
	{
		switch (parameters.type)
		{
			case PlotType::LINE_PLOT:
			{
				const auto *const linePlotParameters = dynamic_cast<const LinePlotParameters*>(&specificParameters);
				if (linePlotParameters == nullptr)
				{
					log(tools::logger::LogLevel::FATAL, "Plot type is LINE_PLOT but the specific parameters are not LinePlotParameters; plot not added.");
					return;
				}
				LinePlot plot(parameters, *linePlotParameters);
				plots[std::make_shared<LinePlot>(plot)] = data;
				break;
			}
			case PlotType::HEATMAP:
			{
				const auto *const heatmapParameters = dynamic_cast<const HeatmapParameters*>(&specificParameters);
				if (heatmapParameters == nullptr)
				{
					log(tools::logger::LogLevel::FATAL, "Plot type is HEATMAP but the specific parameters are not HeatmapParameters; plot not added.");
					return;
				}
				Heatmap plot(parameters, *heatmapParameters);
				plots[std::make_shared<Heatmap>(plot)] = data;
				break;
			}
		}
		log(tools::logger::LogLevel::INFO, std::format("Plot {} added to visualization.", plots.size() - 1));
	}

	void Visualization::plot(const PlotCommonParameters& parameters, const PlotSpecificParameters& specificParameters, const std::string& name, const std::string& component)
	{
		const std::vector<std::pair<std::string, std::string>> dataVec = { {name, component} };
		plot(parameters, specificParameters, dataVec);
	}

	void Visualization::plot(int plotId, const std::vector<std::pair<std::string, std::string>>& data)
	{
		// Find the plot with the specified unique identifier
		const auto it = std::ranges::find_if(plots.begin(), plots.end(), 
			[plotId](const std::pair<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>>& plot)
		{
			return plot.first->getUniqueIdentifier() == plotId;
		});

		// Check if the plot was found
		if (it == plots.end())
		{
			log(tools::logger::LogLevel::ERROR, std::format("Plot with ID {} not found.", plotId));
			return;
		}

		// Add data to the found plot
		plots[it->first].insert(plots[it->first].end(), data.begin(), data.end());
		log(tools::logger::LogLevel::INFO, std::format("Data plotted on plot with ID {}.", plotId));
	}

	void Visualization::plot(const int plotId, const std::string& name, const std::string& component)
	{
		const std::vector<std::pair<std::string, std::string>> dataVec = { {name, component} };
		plot(plotId, dataVec);
	}

	void Visualization::removePlot(int plotId)
	{
		// Find the plot with the specified unique identifier
		const auto it = std::ranges::find_if(plots.begin(), plots.end(), 
			[plotId](const std::pair<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>>& plot)
		{
			return plot.first->getUniqueIdentifier() == plotId;
		});

		if (it != plots.end())
		{
			plots.erase(it);
			log(tools::logger::LogLevel::INFO, std::format("Plot with ID {} removed from visualization.", plotId));
		}
		else
		{
			log(tools::logger::LogLevel::ERROR, std::format("Plot with ID {} not found.", plotId));
		}
	}

	void Visualization::removeAllPlots()
	{
		plots.clear();
		log(tools::logger::LogLevel::INFO, "All plots removed from visualization.");
	}

	void Visualization::removePlottingDataFromPlot(int plotId, const std::pair<std::string, std::string>& data)
	{
		// Find the plot with the specified unique identifier
		const auto it = std::ranges::find_if(plots.begin(), plots.end(),
		[plotId](const std::pair<std::shared_ptr<Plot>, std::vector<std::pair<std::string, std::string>>>& plot)
		{
			return plot.first->getUniqueIdentifier() == plotId;
		});

		// Check if the plot was found
		if (it == plots.end())
		{
			log(tools::logger::LogLevel::ERROR, std::format("Plot with ID {} not found.", plotId));
			return;
		}

		// Check if the data is in the plot
		if (std::ranges::find(plots[it->first].begin(), plots[it->first].end(), data) == plots[it->first].end())
		{
			log(tools::logger::LogLevel::WARNING, std::format("Data '{} - {}' not found in plot {}.", data.first, data.second, plotId));
			return;
		}

		plots[it->first].erase(std::ranges::find(plots[it->first].begin(), plots[it->first].end(), data));
		log(tools::logger::LogLevel::INFO, std::format("Data '{} - {}' removed from plot {}.", data.first, data.second, plotId));
	}

	// If `plot` is a Heatmap and the data contains a "weights" component,
	// derive rows/cols from the element's "input" and "output" component sizes
	// and pass them as a dimension hint so the heatmap renders correctly.
	static void updateHeatmapDimensionHint(
		const std::shared_ptr<Plot>& plot,
		const std::vector<std::pair<std::string, std::string>>& data,
		const std::shared_ptr<Simulation>& simulation)
	{
		auto* heatmap = dynamic_cast<Heatmap*>(plot.get());
		if (heatmap == nullptr) { return;
}

		for (const auto& [elemName, compName] : data)
		{
			if (compName != "weights") { continue;
}
			// rows = input size, cols = output size
			if (simulation->componentExists(elemName, "input") &&
			    simulation->componentExists(elemName, "output"))
			{
				const int rows = static_cast<int>(simulation->getComponentPtr(elemName, "input")->size());
				const int cols = static_cast<int>(simulation->getComponentPtr(elemName, "output")->size());
				heatmap->setDimensionHint(rows, cols);
			}
			break;
		}
	}

	void gatherPlotSeries(const Simulation& simulation,
		const std::vector<std::pair<std::string, std::string>>& sources,
		std::vector<std::vector<double>*>& data,
		std::vector<std::string>& legends)
	{
		// clear() keeps the capacity these buffers already grew to, so a caller
		// that passes the same vectors back every frame stops allocating once
		// the plot count settles.
		data.clear();
		legends.clear();
		data.reserve(sources.size());
		legends.reserve(sources.size());

		for (const auto& [name, component] : sources)
		{
			data.emplace_back(simulation.getComponentPtr(name, component));
			std::string legend = name;
			legend += " - ";
			legend += component;
			legends.emplace_back(std::move(legend));
		}
	}

	void Visualization::renderTile(int plotId)
	{
		const auto it = std::ranges::find_if(plots.begin(), plots.end(),
			[plotId](const auto& p) { return p.first->getUniqueIdentifier() == plotId; });
		if (it == plots.end()) { return;
}

		const auto& data = it->second;
		if (!std::ranges::all_of(data, [this](const std::pair<std::string, std::string>& d)
			{ return simulation->componentExists(d.first, d.second); }))
		{
			removePlot(plotId);
			return;
		}

		updateHeatmapDimensionHint(it->first, data, simulation);
		gatherPlotSeries(*simulation, data, renderDataBuffer, renderLegendBuffer);
		it->first->render(renderDataBuffer, renderLegendBuffer);
	}

	void Visualization::render()
	{
		// Removing a plot while iterating `plots` would invalidate the loop's
		// iterators, so removals are collected and applied after the loop.
		// The buffer is a member purely to reuse its capacity across frames;
		// clear() below is what makes reuse correct.
		plotsToRemoveBuffer.clear();
		for (const auto&[fst, snd] : plots)
		{
			const auto& data = snd;

			// Check if data exists in the simulation, if not, remove it from the plot
			if (!std::ranges::all_of(data, [this](const std::pair<std::string, std::string>& d)
			{
				return simulation->componentExists(d.first, d.second);
				}))
			{
				plotsToRemoveBuffer.push_back(fst->getUniqueIdentifier());
				continue;
			}

			gatherPlotSeries(*simulation, data, renderDataBuffer, renderLegendBuffer);

			const int plotID = fst->getUniqueIdentifier();
			const std::string plotWindowTitle = std::format("Plot #{}##{}", plotID,
				windowSuffix.empty() ? "default" : windowSuffix);

			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(
				ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.47F, vp->WorkPos.y + 52.0F),
				ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(500.0F, 350.0F), ImGuiCond_FirstUseEver);

			const float ui = ImGui::GetIO().FontGlobalScale;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 2.0F * ui));
			ImGui::PushFont(g_BlackLargeFont);
			const bool open = ImGui::Begin(plotWindowTitle.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
			ImGui::PopFont();
			ImGui::PopStyleVar();
			if (open)
			{
				updateHeatmapDimensionHint(fst, data, simulation);
				fst->render(renderDataBuffer, renderLegendBuffer);
			}
			ImGui::End();

			if (!open) {
				plotsToRemoveBuffer.push_back(plotID);
}
		}

		for (const int plotId : plotsToRemoveBuffer) {
			removePlot(plotId);
}
	}
}