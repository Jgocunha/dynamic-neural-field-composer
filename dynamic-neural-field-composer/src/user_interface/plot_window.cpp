// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "./user_interface/plot_window.h"

namespace dnf_composer
{
	namespace user_interface
	{
		PlotWindow::PlotWindow(const std::shared_ptr<Simulation>& simulation)
			: visualization(std::make_shared<Visualization>(simulation))
		{
			PlotParameters parameters;
			parameters.dimensions = { 0, 100, -30, 40 , 1.0};
			parameters.annotations.title = "Plot window ";
			parameters.annotations.x_label = "Spatial dimension";
			parameters.annotations.y_label = "Amplitude";
			createPlot(parameters);
		}

		PlotWindow::PlotWindow(const std::shared_ptr<Simulation>& simulation, PlotParameters parameters)
			: visualization(std::make_shared<Visualization>(simulation))
		{
			createPlot(parameters);
		}

		PlotWindow::PlotWindow(const std::shared_ptr<Visualization>& visualization)
			: visualization(visualization)
		{
			PlotParameters parameters;
			parameters.dimensions = { 0, 100, -30, 40, 1.0 };
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
			for(const auto& plot : plots)
			{
				if(plot.renderDataSelector)
					renderElementSelector(plot);
				renderPlot(plot);
			}
		}

		void PlotWindow::createPlot( PlotParameters& parameters)
		{
			parameters.id = ++current_id;
			parameters.annotations.title += " " + std::to_string(parameters.id);
			plots.emplace_back(parameters);

			const std::string message = "Added a new plot to the application with id: " + parameters.annotations.title + ".\n";
			log(tools::logger::LogLevel::INFO, message);
		}

		void PlotWindow::renderPlot(const PlotParameters& parameters) const
		{
			//configure(parameters.dimensions);

			const std::string plotWindowTitle = parameters.annotations.title + " window";

			if (ImGui::Begin(plotWindowTitle.c_str()))
			{
				ImVec2 plotSize = ImGui::GetContentRegionAvail();  // Get available size in the ImGui window
				plotSize.x -= 5.0f; // Subtract some padding
				plotSize.y -= 5.0f; // Subtract some padding

				if (ImPlot::BeginPlot(parameters.annotations.title.c_str(), plotSize))
				{
					ImPlot::SetupAxes(parameters.annotations.x_label.c_str(), parameters.annotations.y_label.c_str());
					ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_Outside);

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
			ImGui::End();
		}

		void PlotWindow::renderElementSelector(const PlotParameters& parameters) const
		{
			const auto simulation = visualization->getAssociatedSimulationPtr();
			const int numberOfElementsInSimulation = simulation->getNumberOfElements();

			static std::string selectedElementId{};
			static int currentElementIdx = 0;

			const std::string selectorTitle = parameters.annotations.title + " plot selector";

			if (ImGui::Begin(selectorTitle.c_str()))
			{
				//ImGui::Columns(2, nullptr, false); // Use 2 columns

				// First column: List box for selecting an element
				ImGui::Text("Select element");
				if (ImGui::BeginListBox("##Element list", { 300.00f, 200.0f }))
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
				//ImGui::NextColumn();

				std::shared_ptr<element::Element> simulationElement;
				static int currentComponentIdx = 0;
				if (selectedElementId.empty())
				{
					ImGui::Text("Select component");
					if (ImGui::BeginListBox("##Component list", { 250.0f, 200.0f }))
					{
						ImGui::EndListBox();
					}
				}
				else
				{
					simulationElement = simulation->getElement(selectedElementId);
					const std::string elementId = simulationElement->getUniqueName();
					ImGui::Text("Select component");
					if (ImGui::BeginListBox("##Component list", { 250.0f, 200.0f }))
					{
						for (int n = 0; n < static_cast<int>(simulationElement->getComponentList().size()); n++)
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

				// Reset columns
				//ImGui::Columns(1);

				if (ImGui::Button("Add", { 100.0f, 30.0f }))
				{
					visualization->addPlottingData(selectedElementId, simulationElement->getComponentList()[currentComponentIdx]);
				}
			}
			ImGui::End();
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