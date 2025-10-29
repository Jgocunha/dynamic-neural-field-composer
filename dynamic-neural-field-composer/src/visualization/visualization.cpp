// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "visualization/visualization.h"


namespace dnf_composer
{
	Visualization::Visualization(const std::shared_ptr<Simulation>& simulation)
	{
		if (simulation == nullptr)
			throw Exception(ErrorCode::VIS_INVALID_SIM);

		this->simulation = simulation;
		plots = {};
		log(tools::logger::LogLevel::INFO, "Visualization object created.");
	}

	void Visualization::plot(PlotType type)
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

	void Visualization::plot(int plotId, const std::string& name, const std::string& component)
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

	void Visualization::render()
	{
		for (const auto& entry : plots) 
		{
			std::vector<std::pair<std::string, std::string>> data = entry.second;

			// Check if data exists in the simulation, if not remove it from the plot
			if (!std::ranges::all_of(data, [this](const std::pair<std::string, std::string>& d)
			{
				return simulation->componentExists(d.first, d.second);
				}))
			{
				removePlot(entry.first->getUniqueIdentifier());
				return;
			}


			std::vector<std::vector<double>*> allDataToPlotPtr;
			allDataToPlotPtr.reserve(data.size());
			for (const auto& d : data)
			{
				const auto singleDataToPlotPtr = simulation->getComponentPtr(d.first, d.second);
				allDataToPlotPtr.emplace_back(singleDataToPlotPtr);
			}

			std::vector<std::string> legends;
			legends.reserve(data.size());
			for (const auto& d : data)
			{
				legends.emplace_back(d.first + " - " + d.second);
			}

			const int plotID = entry.first->getUniqueIdentifier();
			const std::string plotWindowTitle = "Plot #" + std::to_string(plotID);
			bool open = true;

			if (hasDockTarget) {
				ImGui::SetNextWindowDockID(dockTargetId, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowClass(&dockClass);
			}

			if (ImGui::Begin(plotWindowTitle.c_str(), &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
			{
				entry.first->render(allDataToPlotPtr, legends);
			}
			ImGui::End();

			if (!open)
				removePlot(plotID);
		}

	}
}
