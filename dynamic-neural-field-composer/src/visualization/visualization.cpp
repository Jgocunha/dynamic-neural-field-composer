#include "visualization/visualization.h"

namespace dnf_composer
{
	extern ImFont* g_BlackLargeFont;

	Visualization::Visualization(const std::shared_ptr<Simulation>& simulation)
	{
		if (simulation == nullptr)
			throw Exception(ErrorCode::VIS_INVALID_SIM);

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
		log(tools::logger::LogLevel::INFO, "Plot " + std::to_string(plots.size() - 1) + " added to visualization.");
	}

	void Visualization::plot(const std::vector<std::pair<std::string, std::string>>& data)
	{
		plots[std::make_shared<LinePlot>()] = data;
		log(tools::logger::LogLevel::INFO, "Plot " + std::to_string(plots.size() - 1) + " added to visualization.");
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
				const auto linePlotParameters = dynamic_cast<const LinePlotParameters*>(&specificParameters);
				LinePlot plot(parameters, *linePlotParameters);
				plots[std::make_shared<LinePlot>(plot)] = data;
				break;
			}
			case PlotType::HEATMAP:
			{
				const auto heatmapParameters = dynamic_cast<const HeatmapParameters*>(&specificParameters);
				Heatmap plot(parameters, *heatmapParameters);
				plots[std::make_shared<Heatmap>(plot)] = data;
				break;
			}
		}
		log(tools::logger::LogLevel::INFO, "Plot " + std::to_string(plots.size() - 1) + " added to visualization.");
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
			log(tools::logger::LogLevel::ERROR, "Plot with ID " + std::to_string(plotId) + " not found.");
			return;
		}

		// Add data to the found plot
		plots[it->first].insert(plots[it->first].end(), data.begin(), data.end());
		log(tools::logger::LogLevel::INFO, "Data plotted on plot with ID " + std::to_string(plotId) + ".");
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
			log(tools::logger::LogLevel::INFO, "Plot with ID " + std::to_string(plotId) + " removed from visualization.");
		}
		else
		{
			log(tools::logger::LogLevel::ERROR, "Plot with ID " + std::to_string(plotId) + " not found.");
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
			log(tools::logger::LogLevel::ERROR, "Plot with ID " + std::to_string(plotId) + " not found.");
			return;
		}

		// Check if the data is in the plot
		if (std::ranges::find(plots[it->first].begin(), plots[it->first].end(), data) == plots[it->first].end())
		{
			log(tools::logger::LogLevel::WARNING, "Data '" + data.first + " - " + data.second + "' not found in plot " + std::to_string(plotId) + ".");
			return;
		}

		plots[it->first].erase(std::ranges::find(plots[it->first].begin(), plots[it->first].end(), data));
		log(tools::logger::LogLevel::INFO, "Data '" + data.first + " - " + data.second + "' removed from plot " + std::to_string(plotId) + ".");
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
		if (!heatmap) return;

		for (const auto& [elemName, compName] : data)
		{
			if (compName != "weights") continue;
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

	void Visualization::renderTile(int plotId)
	{
		const auto it = std::ranges::find_if(plots.begin(), plots.end(),
			[plotId](const auto& p) { return p.first->getUniqueIdentifier() == plotId; });
		if (it == plots.end()) return;

		const auto& data = it->second;
		if (!std::ranges::all_of(data, [this](const std::pair<std::string, std::string>& d)
			{ return simulation->componentExists(d.first, d.second); }))
		{
			removePlot(plotId);
			return;
		}

		updateHeatmapDimensionHint(it->first, data, simulation);

		std::vector<std::vector<double>*> ptrs;
		ptrs.reserve(data.size());
		for (const auto& [name, comp] : data)
			ptrs.emplace_back(simulation->getComponentPtr(name, comp));

		std::vector<std::string> legends;
		legends.reserve(data.size());
		for (const auto& [name, comp] : data)
			legends.emplace_back(name + " - " + comp);

		it->first->render(ptrs, legends);
	}

	void Visualization::render()
	{
		for (const auto&[fst, snd] : plots)
		{
			std::vector<std::pair<std::string, std::string>> data = snd;

			// Check if data exists in the simulation, if not, remove it from the plot
			if (!std::ranges::all_of(data, [this](const std::pair<std::string, std::string>& d)
			{
				return simulation->componentExists(d.first, d.second);
				}))
			{
				removePlot(fst->getUniqueIdentifier());
				return;
			}

			std::vector<std::vector<double>*> allDataToPlotPtr;
			allDataToPlotPtr.reserve(data.size());
			for (const auto&[fst, snd] : data)
			{
				const auto singleDataToPlotPtr = simulation->getComponentPtr(fst, snd);
				allDataToPlotPtr.emplace_back(singleDataToPlotPtr);
			}

			std::vector<std::string> legends;
			legends.reserve(data.size());
			for (const auto&[fst, snd] : data)
			{
				legends.emplace_back(fst + " - " + snd);
			}

			const int plotID = fst->getUniqueIdentifier();
			const std::string visible = "Plot #" + std::to_string(plotID);
			const std::string plotWindowTitle = visible + "##" + (windowSuffix.empty() ? "default" : windowSuffix);

			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(
				ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.47f, vp->WorkPos.y + 52.0f),
				ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(500.0f, 350.0f), ImGuiCond_FirstUseEver);

			const float ui = ImGui::GetIO().FontGlobalScale;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 2.0f * ui));
			ImGui::PushFont(g_BlackLargeFont);
			const bool open = ImGui::Begin(plotWindowTitle.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
			ImGui::PopFont();
			ImGui::PopStyleVar();
			if (open)
			{
				updateHeatmapDimensionHint(fst, data, simulation);
				fst->render(allDataToPlotPtr, legends);
			}
			ImGui::End();

			if (!open)
				removePlot(plotID);
		}
	}
}