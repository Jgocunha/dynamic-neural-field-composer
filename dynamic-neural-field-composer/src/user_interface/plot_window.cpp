// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "./user_interface/plot_window.h"

namespace dnf_composer
{
	namespace user_interface
	{
		PlotWindow::PlotWindow(const std::shared_ptr<Visualization>& visualization)
			: visualization(visualization)
		{
			PlotParameters parameters;
			parameters.annotations.title = "Plot window ";
			parameters.annotations.x_label = "Spatial dimension";
			parameters.annotations.y_label = "Amplitude";
			createPlot(parameters);
		}

		PlotWindow::PlotWindow(const std::shared_ptr<Visualization>& visualization, PlotParameters parameters)
			: visualization(visualization)
		{
			createPlot(parameters);
		}

		void PlotWindow::addPlottingData(const std::string& elementId, const std::string& componentId) const
		{
			visualization->addPlottingData(elementId, componentId);
		}

		void PlotWindow::render()
		{
			for (auto it = plots.begin(); it != plots.end();)
			{
				PlotParameters& plot = *it;
				const std::string plotWindowTitle = plot.annotations.title + " window";
				bool open = true; 

				if (ImGui::Begin(plotWindowTitle.c_str(), &open, ImGuiWindowFlags_NoCollapse))
				{
					renderPlot(plot);
					//renderElementSelector(plot);
				}
				ImGui::End();

				if (!open)
				{
					it = plots.erase(it);
					log(tools::logger::LogLevel::INFO, "Plot window closed.");
				}
				else
					++it;
			}
		}

		void PlotWindow::createPlot( PlotParameters& parameters)
		{
			parameters.id = ++current_id;
			//parameters.annotations.title += " " + std::to_string(parameters.id);
			plots.emplace_back(parameters);

			const std::string message = "Added a new plot to the application with id: " + parameters.annotations.title + ".";
			log(tools::logger::LogLevel::INFO, message);
		}

		void PlotWindow::renderPlot(const PlotParameters& parameters) const
		{
			configure(parameters.dimensions);

			ImVec2 plotSize = ImGui::GetContentRegionAvail();  // Get available size in the ImGui window
			plotSize.x -= 5.0f; // Subtract some padding
			plotSize.y -= 5.0f; // Subtract some padding

			static constexpr ImPlotFlags flags = ImPlotFlags_Crosshairs; //| ImPlotFlags_Equal;


			if (ImPlot::BeginPlot(parameters.annotations.title.c_str(), plotSize, flags))
			{
				ImPlot::SetupAxes(parameters.annotations.x_label.c_str(), parameters.annotations.y_label.c_str());
					//ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
				ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Horizontal);

				const int numOfPlots = visualization->getNumberOfPlots();
				for (int j = 0; j < numOfPlots; j++)
				{
					std::string label = visualization->getPlottingLabel(j);
					std::vector<double> data = *visualization->getPlottingData(j);

					// Shift x-values by 1 unit and scale
					std::vector<double> shiftedXValues(data.size());
					for (int i = 0; i < data.size(); ++i) 
						shiftedXValues[i] = (i + 1) * parameters.dimensions.dx;

					ImPlot::PlotLine(label.c_str(), shiftedXValues.data(), data.data(), static_cast<int>(data.size()));
				}

			}
			ImPlot::EndPlot();
		}

		void PlotWindow::renderElementSelector(const PlotParameters& parameters) const
		{
			const auto simulation = visualization->getAssociatedSimulationPtr();
			const int numberOfElementsInSimulation = simulation->getNumberOfElements();

			static std::string selectedElementId{};
			static int currentElementIdx = 0;

			const std::string selectorTitle = parameters.annotations.title + " plot selector";

			if (numberOfElementsInSimulation == 0)
			{
				ImGui::Text("No elements in simulation.");
				return;
			}

			if (ImGui::CollapsingHeader(selectorTitle.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Columns(2, nullptr, false);  // Use 2 columns

				// Calculate the list box height dynamically based on content with a reasonable maximum.
				const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
				float listBoxHeight = std::min(lineHeight * static_cast<float>(numberOfElementsInSimulation), 8 * lineHeight);

				// First column: List box for selecting an element
				ImGui::Text("Select element");
				if (ImGui::BeginListBox("##Element list", ImVec2(-1, listBoxHeight)))
				{
					for (int n = 0; n < numberOfElementsInSimulation; n++)
					{
						const auto element = simulation->getElement(n);
						std::string elementId = element->getUniqueName();
						const bool isSelected = (currentElementIdx == n);
						if (ImGui::Selectable(elementId.c_str(), isSelected))
						{
							selectedElementId = elementId;
							currentElementIdx = n;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}

				// Next column: List box for selecting a component
				ImGui::NextColumn();

				std::shared_ptr<element::Element> simulationElement;
				static int currentComponentIdx = 0;
				if (!selectedElementId.empty())
				{
					simulationElement = simulation->getElement(selectedElementId);
					const int numComponents = static_cast<int>(simulationElement->getComponentList().size());
					listBoxHeight = std::min(lineHeight * static_cast<float>(numComponents), 8 * lineHeight);

					ImGui::Text("Select component");
					if (ImGui::BeginListBox("##Component list", ImVec2(-1, listBoxHeight)))
					{
						for (int n = 0; n < numComponents; n++)
						{
							const bool isSelected = (currentComponentIdx == n);
							if (ImGui::Selectable(simulationElement->getComponentList()[n].c_str(), isSelected))
								currentComponentIdx = n;

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}
				}

				// Reset columns to 1
				ImGui::Columns(1);

				if (ImGui::Button("Add", ImVec2(100.0f, 30.0f)))
				{
					visualization->addPlottingData(selectedElementId, simulationElement->getComponentList()[currentComponentIdx]);
				}
			}
		}


		void PlotWindow::configure(const PlotDimensions& dimensions)
		{
			constexpr static int safeMargin = 1;
			ImPlot::SetNextAxesLimits(dimensions.xMin - safeMargin, dimensions.xMax + safeMargin,
			dimensions.yMin - safeMargin, dimensions.yMax + safeMargin);

			ImPlotStyle& style = ImPlot::GetStyle();
			style.LineWeight = 3.0f;
		}
	}
}